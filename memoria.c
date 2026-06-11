#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "machine.h"
#include "memoria.h"

physical_mem_t memFisica;
virtual_mem_t memVirtual;
pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

int id_table = 0;


unsigned char *translate_dir(const hilo_t *hilo,int va, int *page_fault){
   if(page_fault) *page_fault = 0;
   
   if(!memVirtual.tablas || hilo->PTBR < 0 || hilo->PTBR >= memVirtual.num_tablas){
      if(page_fault) *page_fault = 1;
      return NULL;
   }
   
   page_table_t *tabla = memVirtual.tablas[hilo->PTBR];
   if(!tabla || !tabla->pages || tabla->num_pages <= 0){
      if(page_fault) *page_fault = 1;
      return NULL;
   }
   
   int vpn = va/TAM_PAGE;
   int off = va % TAM_PAGE;
   
   if(vpn < 0 || vpn >=tabla->num_pages || tabla->pages[vpn].valida == 0){
      if(page_fault) *page_fault = 1;
      return NULL;
   }
   
   int frame = tabla->pages[vpn].frame_id;
   int pa = frame*TAM_PAGE +off;
   return memFisica.memoria + pa;
}

void mm_read(hilo_t *hilo, int va, char *out, int *fault)
{
   for(int i=0; i< TAM_PAL; i++){
      unsigned char *pal = translate_dir(hilo, va+i, fault);
      if(!pal || fault) return;
      out[i] = *pal;
   }
}


//Inicializar frames de memoria
static void frame_init()
{
   for(int i=0; i<NUM_FRAMES; i++){
      memFisica.frames[i].libre = 1;
      memFisica.frames[i].pid = -1;
      memFisica.frames[i].pagina = -1;
   }
}
//Si hay capacidad suficiente no se hace nada, si no se aumenta la capacidad
static void hay_capacidad(int nec){
   int new_tam;
   if(memVirtual.tablas == NULL || memVirtual.tam < nec){
      
      if(memVirtual.tam > 0) new_tam = memVirtual.tam;
      else memVirtual.tam = 16;
      while(new_tam < nec) new_tam *= 2;
      
      page_table_t **tmp = (page_table_t **)realloc(memVirtual.tablas, new_tam * sizeof(page_table_t *));
      if(!tmp){
         perror("Error al realloc\n");
         exit(1);
      }
      for(int i=memVirtual.tam; i<new_tam; i++){
         tmp[i] = NULL;
      }
     //Reasignar puntero a array tablas con mayor tamaño
      memVirtual.tablas = tmp;
      memVirtual.tam = new_tam;
   }
}


static int kernel_hay_capacidad(int frames){
  return(memFisica.kernel_next + frames <= KERNEL_FRAMES);
}

static void *kernel_alloc(int frames){
  if(!kernel_hay_capacidad(frames)) return NULL;
  
  void *ptr = memFisica.memoria + memFisica.kernel_next;
  memFisica.kernel_next += frames;
  
  return ptr;
}

int add_ptable(page_table_t *tabla)
{
  if(!tabla) return -1;
   
  int frames_nec = (sizeof(page_table_t) + sizeof(page_t) * tabla->num_pages + TAM_PAGE -1)/TAM_PAGE;
   
  pthread_mutex_lock(&mem_mutex);
   
  if(!kernel_hay_capacidad(frames_nec)){
    pthread_mutex_unlock(&mem_mutex);
    printf("No hay suficiente espacio en la memoria kernel\n");
    return -1;
  }
  
  int base = memFisica.kernel_next;
  memFisica.kernel_next += frames_nec;
  pthread_mutex_unlock(&mem_mutex);
  tabla->id = memVirtual.num_tablas;
  tabla->phys_addr = base;
  
  memVirtual.tablas[memVirtual.num_tablas] = tabla;
  memVirtual.num_tablas++;
  
  return tabla->phys_addr;
}

void virt_mem_init(){
   memVirtual.tam = 16;
   memVirtual.num_tablas = 0;
   memVirtual.tablas = calloc(memVirtual.tam, sizeof(page_table_t *));
   if(!memVirtual.tablas){
      perror("Error al crear el array tablas memoria virtual\n");
      exit(1);
  }
}  

//Inicializar memoria física
void phys_mem_init(){
   pthread_mutex_lock(&mem_mutex);
   
   memFisica.memoria = malloc(TAM_P_MEM*sizeof(unsigned char));
   if(!memFisica.memoria){
      pthread_mutex_unlock(&mem_mutex);
      perror("Error al crear la memoria fisica\n");
      exit(1);
   }
   
   memFisica.frames = malloc(NUM_FRAMES*sizeof(frame_t));
   if(!memFisica.frames){
      pthread_mutex_unlock(&mem_mutex);
      perror("Error al generar los frames de la memoría física\n");
      exit(1);
   }
  
  memFisica.next_frame = 0;
  memFisica.num_tables = 0;
   
  frame_init();
     
  for(int i=0; i< (KERNEL_FRAMES); i++){
    memFisica.frames[i].libre = 0;
    memFisica.frames[i].pid = -1;
    memFisica.frames[i].pagina = -1;
  }
  memFisica.kernel_next = 0;
  pthread_mutex_unlock(&mem_mutex);
}

//Asignar los frames disponibles
int asig_frame_libre(int *frames_libres, int num_page)
{
   if(!frames_libres || num_page <= 0) return -1;
   int frame = 0;
   int cont =0;
   int i;
  
   pthread_mutex_lock(&mem_mutex);

   for(i = 0; i< NUM_FRAMES; i++){
      if(cont >= num_page) break;
      
      if(memFisica.frames[i].libre){
         memFisica.frames[i].libre = 0;
         frames_libres[cont] = i;
         cont++;
      }
   }
     
   if(cont < num_page){
      for(int j=0; j<cont; j++){
         int libre = frames_libres[j];
         memFisica.frames[libre].libre = 1;
      }
      printf("Numero de frames < frames necesarios\n");
      return -1;
   }
   
   pthread_mutex_unlock(&mem_mutex);
   return 0;
}
page_table_t *crear_tabla(int num_pages, const int *frames, int pid)
{
   if(num_pages<=0 || !frames) return NULL;
   
   page_table_t *tabla = (page_table_t *)malloc(sizeof(page_table_t));
   if(!tabla){
      perror("Error al crear la tabla\n");
      exit(1);
   }
   
   tabla->pages= (page_t *)calloc(num_pages, sizeof(page_t));
   if(!tabla->pages){
      perror("Error al crear las páginas de la tabla de páginas\n");
      free(tabla);
      exit(1);
   }
   tabla->num_pages=num_pages;
  
   for(int i=0; i<num_pages; i++){
      int frame = frames[i];
      tabla->pages[i].frame_id = frame;
      printf("    Frame asignado: %d\n", frame);
      tabla->pages[i].valida = 1;
  }
  return tabla;
}

void eliminaet_p_mem(){
   free(memFisica.frames);
   memFisica.frames = NULL;
   free(memFisica.memoria);
   memFisica.memoria = NULL;
   memFisica.next_frame = 0;
   memFisica.num_tables = 0;
}

void printf_tablaPag(page_table_t *tabla){
  for(int i=0; i<tabla->num_pages; i++){
      int frame = tabla->pages[i].frame_id;
      int base = frame*TAM_PAGE;
      for(int j=0; j<(TAM_PAGE/TAM_PAL); j+=TAM_PAL){
         printf("Frame: %d, j: %d \n", frame, j);
         printf("TABLA PAGINAS MEM FISICA %02X %02X %02X %02X\n", memFisica.memoria[base+j], memFisica.memoria[base+j+1],
               memFisica.memoria[base+j+2], memFisica.memoria[base+j+3]);
    }
    if(i == 2) exit(1);
  }
}
