#include "memoria.h"
#include <stdlib.h>

physical_mem_t memFisica;

static void frame_init()
{
   for(int i=0; i<NUM_FRAMES; i++){
      memFisica.frames[i].libre = 1;
      memFisica.frames[i].pid = -1;
      memFisica.frames[i].pagina = -1;
   }
}

void phys_mem_init(){
   memFisica.memoria = malloc(TAM_P_MEM*sizeof(unsigned char));
   if(!memFisica){
      perror("Error al crear la memoria fisica\n");
      exit(1);
   }
   
   memFisica.frames = malloc(NUM_FRAMES*sizeof(frame_t);
   if(!memFisica.frames){
      perror("Error al generar los frames de la memoría física\n");
      exit(1);
   }
   memFisica.next_frame = 0;
   memFisica.num_tables = 0;
   frame_init();
}

int asig_frame_libre(int *frames_libres, int num_page)
{
   if(!frames_libres || num_pages <= 0) return -1;
   
   int frame = memFisica.next_frame;
   int cont = 0;
   int i=0
   
   while(i< NUM_FRAMES && cont < num_pages){
      if(memFisica.frames[i].libre){
         frames_libres[cont++] = frame;
      }
      frame = (frame+1) % NUM_FRAMES;
      i++;
   }
   
   if(cont < num_pages){
      printf("Numero de frames < frames necesarios\n");
      return -1;
   }
   
   memFisica.next_frame = frame;
   return 0;
}
page_table_t *crear_tabla(int num_frames, int *frames, int num_pages)
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
   tabla->pid = pid;
   
   for(int i=0; i<num_pages; i++){
      int frame = frames[i];
      tabla->pages[i].frame_id = frame;
      tabla->pages[i].valida = 1;
      
      memFisica.frames[frame].libre = 0;
      memFisica.frames[frame].pid = pid;
      memFisica.frames[frame]-pagina = i;
  }
  
  memFisica.num_tables++;
  return tabla;
   
}
static void frame_init(){
   for(int i=0; i<NUM_FRAMES; i++){
      memFisica.frames[i].libre = 1;
      memFisica.frames[i].pid = -1;
      memFisica.frames[i].pagina = -1;
   }
}

void phys_mem_init(){
   memFisica.memoria = malloc(TAM_P_MEM);
   memFisica.next_frame = NUM_FRAMES;
   memFisica.frames = malloc(NUM_FRAMES*sizeof(frames_t));
   frame_init();
}

void eliminaet_p_mem(){
   free(memFisica.frames);
   memFisica.frames = NULL;
   free(memFisica.memoria);
   memFisica.memoria = NULL;
   memFisica.nextFrame = 0;
   memFisica.numTables = 0;
}


