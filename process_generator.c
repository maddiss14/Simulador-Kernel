#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"
#include "loader.h"

//Variables globales
pthread_cond_t generator_cond = PTHREAD_COND_INITIALIZER;

void restart_politica(P_FCFS *colaColas){
   for(int i=0; i<colaColas->num_colas; i++){
      if(colaColas->colaPrio[i]!=NULL){
         free(colaColas->colaPrio[i]->lista);
         free(colaColas->colaPrio[i]);
      }
      colaColas->colaPrio[i] = NULL;
   }
   colaColas->num_colas = 0;
   printf("Cola de prioridades reseteada \n");
}

//Inicialización las colas
void politica_initializer(int numPrio, P_FCFS *colaColas)
{
   colaColas->colaPrio = malloc(sizeof(p_queue *) * numPrio);
   colaColas->size = numPrio;

   if(colaColas->colaPrio == NULL){
      perror("Error al crear la cola de colas\n");
      exit(1);
   }
   colaColas->num_colas = 0;
   for(int i=0; i<colaColas->size; i++){
      colaColas->colaPrio[i] = NULL;
   }

   printf("Cola de prioridades creada e inicializada\n");
}

void queue_initializer(int tam, p_queue *queue)
{
   if(queue == NULL){
      perror("queue_initializer: queue es nulo\n");
      exit(1);
   }

   queue->lista = malloc(sizeof(PCB *)* tam);
   if(queue->lista == NULL){
      perror("Error al crear la lista de punteros \n");
      exit(1);
   }

   for (int i=0; i < tam; i++){
      queue->lista[i]=NULL;
   }

   queue->size = tam;
   queue->num_process = 0;
   queue->first = NULL;
   queue->last = NULL;
}

void eliminate_queues(P_FCFS *colaColas){
   int pid;
   int prio;

   if(colaColas->colaPrio != NULL){
      for(int i=0; i<colaColas->num_colas; i++){
         printf("Cola %d:  \n", i);

         if(colaColas->colaPrio[i]!=NULL){
            p_queue *colaProces = colaColas->colaPrio[i];

            for(int j=0; j<colaProces->num_process; j++){
               PCB *actual = colaProces->lista[j];
               pid = actual->pid;
               prio = actual->prio;

               free(actual);
               printf("   Proceso liberado %d, prioridad %d \n", pid, prio);
            }

            free(colaProces->lista);
            free(colaProces);
            printf("Cola %d liberada\n\n", i);
         }
      }
   }
}

//Liberar memoria
void eliminate_politica(P_FCFS *colaColas)
{
   eliminate_queues(colaColas);

   free(colaColas->colaPrio);
   printf("Colas de prioridades liberadas\n");
}

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
            printf("Lista de procesos añadida a la cola de prioridades posicion %d \n", i);
            return;
         }else{
            printf("Cola de prioridades está llena no se ha podido añadir la nueva cola\n");
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
      printf("Nueva cola creada al final\n");
   }
   else{
      printf("No se puede añadir una nueva cola\n");
      free(proceso);
   }
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
      printf("Ruta %s\n", ruta);
      read_prog(ruta, nuevo);
      nuevo->pid=pid_gen++;
      nuevo->vida= rand() % 10 + 7;
      if (r_colaColas.size > 0) {
         nuevo->prio = rand() % r_colaColas.size;
      } else {
         nuevo->prio = 0;
      }
      add_process(nuevo, &r_colaColas);
      pthread_mutex_unlock(&clock_mutex);
   }
}
