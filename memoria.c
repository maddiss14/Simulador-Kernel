#include "memoria.h"
#include <stdlib.h>
#include <stdio.h>

physical_mem_t memFisica;
pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;
int id_table = 0;

//Inicializar frames de memoria
static void frame_init()
{
   for(int i=0; i<NUM_FRAMES; i++){
      memFisica.frames[i].libre = 1;
      memFisica.frames[i].pid = -1;
      memFisica.frames[i].pagina = -1;
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
   
   pthread_mutex_unlock(&mem_mutex);
}

//Asignar los frames disponibles
int asig_frame_libre(int *frames_libres, int num_page)
{
   if(!frames_libres || num_page <= 0) return -1;
   int frame;
   int cont, i;
  
   pthread_mutex_lock(&mem_mutex);
   frame = memFisica.next_frame;
   cont = 0;
   i=0;
   while(i< NUM_FRAMES && cont < num_page){
      if(memFisica.frames[i].libre){
         memFisica.frames[i].libre = 0;
         frames_libres[cont++] = frame;
         cont++;
      }
      frame = (frame+1) % NUM_FRAMES;
      i++;
   }
   pthread_mutex_unlock(&mem_mutex);
   
   if(cont < num_page){
      pthread_mutex_lock(&mem_mutex);
      for(int j=0; j<cont; j++){
         int libre = frames_libres[j];
         memFisica.frames[libre].libre = 1;
      }
      printf("Numero de frames < frames necesarios\n");
      pthread_mutex_unlock(&mem_mutex);
      return -1;
   }
   memFisica.next_frame = frame;
   return 0;
}
page_table_t *crear_tabla(int num_pages, const int *frames)
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
   tabla->id = id_table;
   id_table++;
   for(int i=0; i<num_pages; i++){
      int frame = frames[i];
      tabla->pages[i].frame_id = frame;
      tabla->pages[i].valida = 1;
      
      memFisica.frames[frame].libre = 0;
      memFisica.frames[frame].pid = 0;
      memFisica.frames[frame].pagina = i;
  }
  
  memFisica.num_tables++;
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


