#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "memoria.h"
#include "process_generator.h"

pthread_cond_t loader_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t loader_mutex = PTHREAD_MUTEX_INITIALIZER;


void read_prog(char *filename, PCB *proceso)
{
   struct stat st;
   int tam_fich;
   int val_text, tam_text, pag_text;
   int val_data, tam_data, pag_data;
   int pag_tot;
   char *linea;
   int *frames;
   int aux,cont;
   char *buf, *buf_cp;
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

   buf_cp = strndup(buf, tam_fich);
   if(!buf_cp){
      perror("Error al copiar el bufer\n");
      free(buf);
      exit(1);
   }
   
   linea = strtok(buf, "\n"); //Separar por lineas
   if(!linea){
      perror("No se ha podido leer cabecera .txt\n");
      free(buf_cp);
      free(buf);
      exit(1);
   }
   sscanf(linea+6, "%x", &val_text);
   printf("Valor .text %d (0x%06X)\n", val_text, val_text);
   
   linea=strtok(NULL, "\n");
   if(!linea){
      perror("No se ha podido leer la cabecera .dat\n");
      free(buf_cp);
      free(buf);
   }
   
   sscanf(linea + 6, "%x", &val_data);
   printf("Valor .data %d (0x%06X)\n", val_data, val_data);

   linea=strtok(NULL, "\n");
   tam_text = val_data - val_text;
   tam_data = tam_fich-val_data;
    
   if(val_data < 0 || val_text < 0){
      perror("Offsets inválidos tam_data\n");
      free(buf);
      free(buf_cp);
      exit(1);
   }
   pag_text = (tam_text + TAM_PAGE-1) / TAM_PAGE;
   pag_data = (tam_data + TAM_PAGE-1) / TAM_PAGE;
   pag_tot = pag_text + pag_data;

   frames = (int *)malloc(pag_tot * sizeof(int));
   if(!frames){
      perror("Error reservar frames\n");
      free(buf);
      free(buf_cp);
      exit(1);
   }
   printf("PAG_TOT %d\n", pag_tot);
   if(asig_frame_libre(frames, pag_tot)== -1){
      perror("Error al asignar los frames libres\n");
      free(frames);
      free(buf);
      free(buf_cp);
      exit(1);
   }
   page_table_t *tabla = crear_tabla(pag_tot, frames, proceso->pid);
   if(!tabla){
      perror("Error al crear la tabla de páginas\n");
      free(frames);
      free(buf_cp);
      free(buf);
      exit(1);
   }
   int pgb = add_ptable(tabla);
   if(pgb < 0){
      perror("Error al añadir la tabla de páginas\n");
      free(frames);
      free(buf_cp);
      free(buf);
      exit(1);
   }
   pthread_mutex_lock(&mem_mutex);
  
   //Actualizar información del PCB
   for(int i=0; i<pag_text; i++){
      int frame = frames[i];
      copied = 0;
      while(linea && copied < TAM_PAGE && copied < tam_text){
         //Copiar como hex no ASCII
         for(int j=0; j<TAM_PAL; j++){
            sscanf(linea+(j*2), "%2hhx", &instr[j]);
         }
         memcpy(memFisica.memoria + frame*TAM_PAGE + copied, instr, TAM_PAL);
         printf("INSTR = %02X %02X %02X %02X\n", instr[0], instr[1], instr[2], instr[3]);
         printf("MEMORIA = %02X %02X %02X %02X\n", memFisica.memoria[frame*TAM_PAGE+copied], memFisica.memoria[frame*TAM_PAGE+copied+1], 
                 memFisica.memoria[frame*TAM_PAGE+copied+2], memFisica.memoria[frame*TAM_PAGE+copied+3]);
         copied += TAM_PAL;
         linea = strtok(NULL, "\n");
      }
         memFisica.frames[frame].libre = 0;
         memFisica.frames[frame].pid = proceso->pid;
         memFisica.frames[frame].pagina = i;
   }

   
   linea = strtok(NULL, "\n");
   for(int i=0; i<pag_data; i++){
      int frame = frames[pag_text + i];
      copied = 0;
      while(linea && copied < TAM_PAGE && copied < tam_data){
         //Copiar como hex no ASCII
         for(int j=0; j<TAM_PAL; j++){
            sscanf(linea+(j*2), "%2hhx", &instr[j]);
         }
         memcpy(memFisica.memoria + frame*TAM_PAGE + copied, instr, TAM_PAL);
         printf("MEMORIA = %d\n", memFisica.memoria[frame*TAM_PAGE]);
         printf("DATA = %02X %02X %02X %02X\n", instr[0], instr[1], instr[2], instr[3]);
         copied += TAM_PAL;
         linea = strtok(NULL, "\n");
         memFisica.frames[frame].libre = 0;
         memFisica.frames[frame].pid = proceso->pid;
         memFisica.frames[frame].pagina = i;
      }
   }
   pthread_mutex_unlock(&mem_mutex);
   
   proceso->mm.code = 0;
   proceso->mm.data = tam_text;
   proceso->mm.pgb = pgb;

   free(frames);
   free(buf_cp);
   free(buf);
}
