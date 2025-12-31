#pragma once

#include <pthread.h>

extern int frecMin_pGen;
extern int frecMax_pGen;

typedef struct PCB{
   int pid;
   int vida;
   int prio;
}PCB;

typedef struct p_queue{
   PCB **lista;
   PCB *first;
   PCB *last;
   int size;
   int num_process;
}p_queue;

typedef struct P_FCFS{
   p_queue **colaPrio;
   int size;
   int q;
   int num_colas;
}P_FCFS;

extern pthread_cond_t generator_cond;
extern p_queue queue;

void queue_initializer(int size, p_queue *queue);
void encolar_proceso(PCB *proceso);
void *generator_thread(void *arg);
void eliminate_queue();
void politica_initializer(int tam);

void eliminate_queue();
