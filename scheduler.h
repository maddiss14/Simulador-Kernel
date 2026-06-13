#pragma once

#include <pthread.h>
#include <stdint.h>
#include "process_generator.h"
#include "machine.h"

typedef struct PCB PCB;

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

extern pthread_cond_t scheduler_cond;
extern p_queue queue;
extern P_FCFS r_colaColas;
extern P_FCFS f_colaColas;

void queue_initializer(int tam, p_queue *queue);
void eliminate_queues(P_FCFS *colaColas);
void politica_initializer(int numPrio, P_FCFS *colaColas);
//Liberar memoria
void eliminate_politica(P_FCFS *colaColas);
void *scheduler_thread(void *arg);
