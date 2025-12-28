#pragma once

#include <pthread.h>

extern int frecMin_pGen;
extern int frecMax_pGen;

typedef struct PCB{
   int pid;
   int vida;
}PCB;

typedef struct p_queue{
   PCB **lista;
   PCB *first;
   PCB *last;
   int size;
   int num_process;
}p_queue;

extern pthread_cond_t generator_cond;
extern p_queue lista;

void queue_initializer(int size);
void encolar_proceso(PCB *proceso);
void *generator_thread(void *arg);

void eliminate_queue();
