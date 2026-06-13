#pragma once

#include <pthread.h>
#include <stdint.h>
#include "scheduler.h"


extern int frecMin_pGen;
extern int frecMax_pGen;

typedef struct P_FCFS P_FCFS;

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
   
   int PC;
   int regs[16];
}PCB;

extern pthread_cond_t generator_cond;

void add_process(PCB *proceso, P_FCFS *colaColas);
void *generator_thread(void *arg);
PCB *sig_process(P_FCFS *colaColas);

