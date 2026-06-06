#pragma once

#include <pthread.h>

extern int frecMin_pGen;
extern int frecMax_pGen;

typedef struct{
   uint32_t code;
   uint32_t data;
   uint32_t pgb;
} mm_t;

typedef struct PCB{
   int pid;
   int vida;
   int prio;
   mm_t mm;
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
extern P_FCFS r_colaColas;
extern P_FCFS f_colaColas;

void queue_initializer(int size, p_queue *queue);
void add_process(PCB *proceso, P_FCFS *colaColas);
void *generator_thread(void *arg);
void eliminate_queue(P_FCFS *colaColas);
void eliminate_politica(P_FCFS *colaColas);
void restart_politica(P_FCFS *colaColas);
void politica_initializer(int numPrio, P_FCFS *colaColas);
PCB *sig_process(P_FCFS *colaColas);
void eliminate_queue(P_FCFS *colaColas);
