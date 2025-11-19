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
      printf("Error al crear la lista de punteros \n");
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
         while(actual!=NULL){
            queue.lista[i]== NULL;
            printf("Proceso liberado %d\n", actual->pid);
            free(actual);
            i++;
      }
      free(queue.lista);
      queue.first = NULL;
      queue.last=NULL;
   }
}

void encolar_proceso(PCB *proceso){
   if(queue.num_process == queue.size){
      printf("Error al encolar proceso %d la cola esta llena \n", proceso->pid);
   }else{
      if(queue.first==NULL){
         queue.first=proceso;
      }
      queue.lista[queue.num_process+1]=proceso;
      queue.last = proceso;
      queue.num_process++;
      printf("Proceso %d generado \n", proceso->pid);
   }
}


void *generator_thread(void *arg){
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&generator_cond, &clock_mutex);
      PCB *nuevo = malloc(sizeof(PCB));
      if(nuevo == NULL){
         printf("Error al crear el proceso/n");
         exit(1);
      }
      nuevo->pid=pid_gen++;
      encolar_proceso(nuevo);
   }
}
