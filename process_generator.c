#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"

P_FCFS  colaColas;

pthread_cond_t generator_cond = PTHREAD_COND_INITIALIZER;
int pid_gen=0;

void politica_initializer(int numPrio){
   colaColas.colaPrio = malloc(sizeof(p_queue *)* numPrio);
   colaColas.size = numPrio;
   colaColas.num_colas = 0;
   if(colaColas.colaPrio == NULL){
      perror("Error al crear la cola de colas\n");
      exit(1);
   }
   for(int i=0; i<numPrio; i++){
      colaColas.colaPrio[i] = NULL;
   }
   printf("Cola de prioridades generada \n");
}

void queue_initializer(int tam, p_queue *queue){
   queue->lista = malloc(sizeof(PCB *)* tam);
   queue->size = tam;
   queue->num_process=0;
   if(queue->lista == NULL){
      perror("Error al crear la lista de punteros \n");
      exit(1);
   }
   for (int i=0; i < tam; i++){
      queue->lista[i]=NULL;
   }
   queue->first = NULL;
   queue->last = NULL;
   printf("Lista de procesos generada \n");
}

void eliminate_queue(){
   int pid;
   if(colaColas.colaPrio != NULL){
      for(int i=0; i<colaColas.num_colas; i++){
         if(colaColas.colaPrio[i]!=NULL){
	    p_queue *colaProces = colaColas.colaPrio[i];
	    for(int j=0; j<colaProces->num_process; j++){
	       PCB *actual = colaProces->lista[j];
               pid = actual->pid;
	       free(actual);
               printf("Proceso liberado %d\n", pid);
            }
	    printf("Cola liberada %d\n", i);
	    free(colaProces);
         }
      }
      free(colaColas.colaPrio);
   }
}

void add_process(PCB *proceso){
   if(colaColas.num_colas == 0){
      p_queue *queue;
      queue_initializer(5, queue);
      colaColas.colaPrio[0] = queue;
      colaColas.num_colas = 1;
      queue->first = proceso;
      queue->last = proceso;
      queue->num_process = 1;
      printf("Lista de procesos añadida a la cola de prioridades");
      printf("Proceso %d con vida %d añadido", proceso->pid, proceso->vida);
   }
   else {
      for(int i=0; i< colaColas.num_colas; i++){
         p_queue *actual = colaColas.colaPrio[i];
         if(actual->lista[0]->pid < proceso->pid){
            if(colaColas.num_colas < colaColas.size){
               memmove(colaColas.colaPrio[i+1], colaColas.colaPrio[i], (colaColas.num_colas - 1)*sizeof(P_FCFS));
               p_queue *nuevo;
	       queue_initializer(5, nuevo);
   	       colaColas.colaPrio[i] = nuevo;
               printf("Lista de procesos añadida a la cola de prioridades posicion %d \n", i);
            }else{
               printf("Cola de prioridades está llena no se ha podido añadir la nueva cola");
	       free(proceso);
	    }
         }
         if(actual->lista[0]->pid == proceso->pid){
            if(actual->num_process == 0){
	       actual->first = proceso;
	    }
	    if(actual->num_process < actual->size){
	       actual->last = proceso;
	       actual->lista[actual->num_process+1] = proceso;
	       actual->num_process += 1;
               printf("Proceso %d añadido con vida: %d \n", proceso->pid, proceso->vida);
	    }else{
	       printf("Error al añadir el proceso la cola está llena");
	       free(proceso);
	    }
            break;
         }
      }
   }
}

void *generator_thread(void *arg){
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&generator_cond, &clock_mutex);
      PCB *nuevo = malloc(sizeof(PCB));
      if(nuevo == NULL){
         perror("Error al crear el proceso/n");
         exit(1);
      }
      nuevo->pid=pid_gen++;
      nuevo->vida= rand() % 10 + 1;
      nuevo->prio = rand()% 10+1;
      add_process(nuevo);
      pthread_mutex_unlock(&clock_mutex);
   }
}
