#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "memoria.h"
#include "process_generator.h"

void read_prog(char *filename, PCB *proceso)
{
   struct stat st;
   int tam_fich;
   int val_text, tam_text, pag_text;
   int val_data, tam_data, pag_data;
   int pag_tot;
   char *linea;
   int *frames, frame;
   int aux,cont;
   char *buf;
   size_t leido;
   int chunk;
   unsigned char instr[TAM_PAL] = {0};
   int copied;

   if(stat(filename, &st) == -1){
      perror("Error al intentar obtener el tamaño del fichero\n");
      exit(1);
   }

   tam_fich = (int)st.st_size;
   if(tam_fich==0){
      perror("Tamaño del fichero es 0\n");
      exit(1);
   }

   FILE *file =fopen(filename, "rb");
   if(!file){
      perror("Error al abrir el fichero\n");
      exit(1);
   }
   
   buf = (char *)malloc(tam_fich);

   leido = fread(buf, 1, tam_fich, file);
   fclose(file);
   if(leido != tam_fich){
      perror("Error al leer el fichero completo\n");
      free(buf);
      exit(1);
   }

   linea = strtok(buf, "\n"); //Separar por lineas
   if(!linea){
      perror("No se ha podido leer cabecera .txt\n");
      free(buf);
      exit(1);
   }
   sscanf(linea+6, "%x", &val_text);
   printf("Valor .text %d (0x%06X)\n", val_text, val_text);
   
   linea=strtok(NULL, "\n");
   if(!linea){
      perror("No se ha podido leer la cabecera .dat\n");
      free(buf);
   }
   
   sscanf(linea + 6, "%x", &val_data);
   printf("Valor .data %d (0x%06X)\n", val_data, val_data);

   linea=strtok(NULL, "\n");
   tam_text = (val_data+12) - val_data;
   tam_data = (tam_text+12) - val_text;
   printf("tam_data: %d, tam_text %d \n", tam_data, tam_text);
   if(val_data < 0 || val_text < 0){
      perror("Offsets inválidos tam_data\n");
      free(buf);
      exit(1);
   }
   pag_tot = (tam_text + tam_data + TAM_PAGE-1) / TAM_PAGE;
   

   frames = (int *)malloc(pag_tot * sizeof(int));
   if(!frames){
      perror("Error reservar frames\n");
      free(buf);
      exit(1);
   }
   printf("PAG_TOT %d\n", pag_tot);
   if(asig_frame_libre(frames, pag_tot)== -1){
      perror("Error al asignar los frames libres\n");
      free(frames);
      free(buf);
      exit(1);
   }
   page_table_t *tabla = crear_tabla(pag_tot, frames, proceso->pid);
   if(!tabla){
      perror("Error al crear la tabla de páginas\n");
      free(frames);
      free(buf);
      exit(1);
   }
   int pgb = add_ptable(tabla);
   if(pgb < 0){
      perror("Error al añadir la tabla de páginas\n");
      free(frames);
      free(buf);
      exit(1);
   }
   pthread_mutex_lock(&mem_mutex);
  
   //Actualizar información del PCB

  copied = 0;
  while(copied < (tam_text+tam_data)){
    //Copiar como hex no ASCII
    for(int i=0; i<TAM_PAL; i++){
      sscanf(linea+(i*2), "%2hhx", &instr[i]);
    }
    frame = frames[copied/TAM_PAGE];
    memcpy(memFisica.memoria + frame*TAM_PAGE + copied, instr, TAM_PAL);
    printf("COPIED = %02X %02X %02X %02X\n", memFisica.memoria[frame*TAM_PAGE+copied], memFisica.memoria[frame*TAM_PAGE+copied+1], 
          memFisica.memoria[frame*TAM_PAGE+copied+2], memFisica.memoria[frame*TAM_PAGE+copied+3]);
    printf(" Dirección que uso para acceder: %d\n Copied: %d, Frame: %d\n", frame*TAM_PAGE+copied, copied, frame);
    copied += TAM_PAL;
    linea = strtok(NULL, "\n");
  }
  
  for(int i=0; i<(copied/TAM_PAGE); i++){
    memFisica.frames[frame].libre = 0;
    memFisica.frames[frame].pid = proceso->pid;
    memFisica.frames[frame].pagina = i;
  } 
  
  pthread_mutex_unlock(&mem_mutex);
   
  proceso->mm.code = val_text;
  proceso->mm.data = val_data;
  proceso->mm.pgb = pgb;

  free(frames);
  free(buf);
}
