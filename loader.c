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
   
   int remaining = tam_text;
   int copied = 0;
   for(int i=0; i<pag_text; i++){
      int frame = frames[i];
      int chunk = remaining > TAM_PAGE ? TAM_PAGE : remaining;
      if(chunk > 0){
         memcpy(memFisica.memoria + frame*TAM_PAGE, buf_cp + val_text + copied, (size_t)chunk);
      }
      memFisica.frames[frame].libre = 0;
      memFisica.frames[frame].pid = proceso->pid;
      memFisica.frames[frame].pagina = i;
      copied+=chunk;
      remaining -= chunk;
   }

   remaining = tam_data;
   copied = 0;
   for(int i=0; i<pag_data; i++){
      int frame = frames[i];
      int chunk = remaining > TAM_PAGE ? TAM_PAGE : remaining;
      if(chunk > 0){
         memcpy(memFisica.memoria + frame*TAM_PAGE, buf_cp + val_data + copied, (size_t)chunk);
      }
      memFisica.frames[frame].libre = 0;
      memFisica.frames[frame].pid = proceso->pid;
      memFisica.frames[frame].pagina = i;
      copied+=chunk;
      remaining -= chunk;
   }
   pthread_mutex_unlock(&mem_mutex);
   
   proceso->mm.code = 0;
   proceso->mm.data = tam_text;
   proceso->mm.pgb = pgb;

   free(frames);
   free(buf_cp);
   free(buf);
}
