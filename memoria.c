#include "memoria.h"
#include <stdlib.h>

physical_mem_t memFisica;

void crear_tabla(int num_frames, int *frames)
{
   page_table_t *tabla = malloc(
   for(int i=0; i<num_frames; i++){
      
   
}
int void asig_frame_libre(int *frames_libres, int num_page)
{
   int frame = memFisica.next_frame;
   int cont = 0;
   int i=0
   while(i< NUM_FRAMES && cont < num_pages){
      if(memFisica.frames[i].libre){
         frames_libres[cont] = frame;
         cont++;
      }
      frame = (frame+1) % NUM_FRAMES;
      i++;
   }
   if(cont < num_pages){
      printf("Numero de frames < frames necesarios\n");
      return -1;
   }
   memFisica.next_frame = frame;
   return 1;
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
}


