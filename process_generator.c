#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <memory.h>

#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"
#include "process_manager.h"
#include "memoria.h"

//Variables globales
pthread_cond_t generator_cond = PTHREAD_COND_INITIALIZER;

PCB *sig_process(P_FCFS *colaColas)
{
   if(colaColas->colaPrio != NULL){

      for(int i = 0; i < colaColas->num_colas; i++){
         p_queue *queue = colaColas->colaPrio[i];

         if(queue->num_process > 0){
            PCB *process = queue->first;

            for(int j = 1; j<queue->num_process; j++){
               queue->lista[j-1] = queue->lista[j];
            }

            queue->num_process--;
            queue->lista[queue->num_process] = NULL;

            if(queue->num_process > 0){
               queue->first = queue->lista[0];
               queue->last = queue->lista[queue->num_process - 1];
            } else {
               queue->first = NULL;
               queue->last = NULL;
            }
            return process;
         }
      }
   }
   return NULL;
}

//Función auxiliar para crear e inicializar colas
static p_queue *crear_queue()
{
   p_queue *queue = malloc(sizeof(p_queue));
   if(queue == NULL){
      perror("crear_queue: malloc fallo\n");
      exit(1);
   }
   queue_initializer(5, queue);
   return queue;
}

//Añadir proceso a la cola
static void anadir_process(p_queue *queue, PCB *proceso)
{
  if(queue->num_process < queue->size){

     if(queue->num_process == 0){
        queue->first = proceso;
     }

     queue->last = proceso;
     queue->lista[queue->num_process] = proceso;
     queue->num_process++;

     printf("Proceso %d con vida: %d y prioridad: %d añadido\n",
            proceso->pid, proceso->vida, proceso->prio);
   }
   else{
      printf("La cola de procesos está llena, no se ha podido añadir el proceso %d\n",
             proceso->pid);
      free(proceso);
   }
}

void add_process(PCB *proceso, P_FCFS *colaColas)
{
   //Cola prioridades está vacía
   if(colaColas->num_colas == 0){

      p_queue *queue = crear_queue();

      colaColas->colaPrio[0] = queue;
      colaColas->num_colas++;

      anadir_process(queue, proceso);
      return;
   }

   //Todas las prioridades tienen una cola asignada
   if(colaColas->num_colas == colaColas->size){
      p_queue *queue = colaColas->colaPrio[proceso->prio];
      anadir_process(queue, proceso);
      return;
   }

   for(int i=0; i< colaColas->num_colas; i++){
      p_queue *actual = colaColas->colaPrio[i];

      //Insertar nueva cola de procesos al principio
      if(actual->num_process > 0 &&
         actual->lista[0]->prio > proceso->prio){

         //Insertar nueva cola posición i
         if(colaColas->num_colas < colaColas->size){

            //Mover colas una posición a la derecha
            for(int j=colaColas->num_colas; j>i; j--){
               colaColas->colaPrio[j] = colaColas->colaPrio[j-1];
            }

            p_queue *nuevo = crear_queue();

            colaColas->colaPrio[i] = nuevo;
            colaColas->num_colas++;

            anadir_process(nuevo, proceso);
            //printf("Lista de procesos añadida a la cola de prioridades posicion %d \n", i);
            return;
         }else{
            //printf("Cola de prioridades está llena no se ha podido añadir la nueva cola\n");
            free(proceso);
            return;
         }
      }
      //Misma prioridad
      else if(actual->num_process > 0 &&
              actual->lista[0]->prio == proceso->prio){
        anadir_process(actual, proceso);
        return;
      }
   }

   //Nueva cola al final
   if(colaColas->num_colas < colaColas->size){

      p_queue *nuevo = crear_queue();

      colaColas->colaPrio[colaColas->num_colas] = nuevo;
      colaColas->num_colas++;

      anadir_process(nuevo, proceso);
      //printf("Nueva cola creada al final\n");
   }
   else{
      //printf("No se puede añadir una nueva cola\n");
      free(proceso);
   }
}

static void read_prog(char *filename, PCB *proceso)
{
   struct stat st;
   int tam_fich;
   int val_text, tam_text, pag_text;
   int val_data, tam_data, pag_data;
   int pag_tot;
   char *linea;
   int *frames, frame;
   int aux,cont;
   char *buf, *bufcp;
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
  
  bufcp = strdup(buf);
  int n_lineas = 0;
  char *tmp = strtok(bufcp, "\n");
  while(tmp){
    n_lineas++;
    tmp = strtok(NULL, "\n");
  }
  free(bufcp);
  
   linea = strtok(buf, "\n"); //Separar por lineas
   if(!linea){
      perror("No se ha podido leer cabecera .txt\n");
      free(buf);
      exit(1);
   }
   sscanf(linea+6, "%x", &val_text);
   //printf("Valor .text %d (0x%06X)\n", val_text, val_text);
   
   linea=strtok(NULL, "\n");
   if(!linea){
      perror("No se ha podido leer la cabecera .dat\n");
      free(buf);
   }
   
   sscanf(linea + 6, "%x", &val_data);
   //printf("Valor .data %d (0x%06X)\n", val_data, val_data);

   linea=strtok(NULL, "\n");
   tam_text = val_data;
   tam_data = (n_lineas*4) - tam_text - 8;
   
   //printf("tam_data: %d, tam_text%d , tam_fich%d \n", tam_data, tam_text, n_lineas);
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
   //printf("PAG_TOT %d\n", pag_tot);
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
  while(linea && copied < (tam_text+tam_data)){
    //Copiar como hex no ASCII
    sscanf(linea, "%2hhx%2hhx%2hhx%2hhx", &instr[0], &instr[1], &instr[2], &instr[3]);

    frame = frames[copied/TAM_PAGE];
    int offset = copied % TAM_PAGE;
    memcpy(memFisica.memoria + frame*TAM_PAGE + offset, instr, TAM_PAL);
    //printf("COPIED = %02X %02X %02X %02X\n", memFisica.memoria[frame*TAM_PAGE+copied], memFisica.memoria[frame*TAM_PAGE+copied+1],  memFisica.memoria[frame*TAM_PAGE+copied+2], memFisica.memoria[frame*TAM_PAGE+copied+3]);
    //printf(" Dirección que uso para acceder: %d\n Copied: %d, Frame: %d\n", frame*TAM_PAGE+copied, copied, frame);
    copied += TAM_PAL;
    linea = strtok(NULL, "\n");
  }
  
  for(int i=0; i<(copied/TAM_PAGE); i++){
    int f = frames[i];
    memFisica.frames[f].libre = 0;
    memFisica.frames[f].pid = proceso->pid;
    memFisica.frames[f].pagina = i;
  } 
  
  pthread_mutex_unlock(&mem_mutex);
   
  proceso->mm.code = val_text;
  proceso->mm.data = val_data;
  proceso->mm.pgb = pgb;

  free(frames);
  free(buf);
}

//Hilo de process generator
void *generator_thread(void *arg){
   int pid_gen = 0;
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&generator_cond, &clock_mutex);
      PCB *nuevo = malloc(sizeof(PCB));
      char ruta[4096];
      snprintf(ruta, sizeof(ruta), "../prometheus/prog%03d.elf", pid_gen);
      //printf("Ruta %s\n", ruta);
      nuevo->pid=pid_gen++;
      nuevo->vida= rand() % 10 + 7;
      nuevo->PC = -1;
      read_prog(ruta, nuevo);
      if (r_colaColas.size > 0) {
         nuevo->prio = rand() % r_colaColas.size;
      } else {
         nuevo->prio = 0;
      }
      add_process(nuevo, &r_colaColas);
      pthread_mutex_unlock(&clock_mutex);
   }
}
