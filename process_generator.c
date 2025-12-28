#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"

p_queue queue;

pthread_cond_t generator_cond = PTHREAD_COND_INITIALIZER;
int pid_gen=0;
int t_frecuencia=4;

void queue_initializer(int tam){
   queue.lista = malloc(sizeof(PCB *)* tam);
   queue.size = tam;
   queue.num_process=0;
   if(queue.lista == NULL){
      perror("Error al crear la lista de punteros \n");
      exit(1);
   }
   for (int i=0; i < tam; i++){
      queue.lista[i]=NULL;
   }
   queue.first = NULL;
   queue.last = NULL;
   printf("Lista de procesos generada \n");
}

void eliminate_queue(){
   if(queue.lista != NULL){
         PCB *actual = queue.first;
         int i=0;
         while(actual!=NULL && queue.size){ 
            actual = queue.lista[i];
	    queue.lista[i] = NULL;
            printf("Proceso liberado %d\n", actual->pid);
            free(actual);
            i++;
      }
      free(queue.lista);
      queue.first = NULL;
      queue.last=NULL;
   }
}

void add_process(PCB *proceso){
   if(queue.num_process == queue.size){
      printf("Error al encolar proceso %d la cola esta llena \n", proceso->pid);
      free(proceso);
   }else{
      if(queue.first==NULL){
         queue.first=proceso;
      }
      queue.lista[queue.num_process]=proceso;
      queue.last = proceso;
      queue.num_process++;
      printf("Proceso %d generado vida: %d\n", proceso->pid, proceso->vida);
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
      add_process(nuevo);
      pthread_mutex_unlock(&clock_mutex);

   }
}
