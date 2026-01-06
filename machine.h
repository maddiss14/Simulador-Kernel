#pragma once

#include "process_generator.h"
#include "memoria.h"

#define QUANTUM 2

typedef struct{
   int val;
   int pres;
   int ref;
   int dirty;
} TLB;

typedef struct{
   TLB *tlb;
} MMU;

typedef struct{
   int id_hilo;
   PCB *r_pcb;
   int quantum;
   int estado; // 0 preparado 1 ejecutando 2 finalizado
   int PC;
   int PTBR;
   MMU mmu;
} hilo_t;

typedef struct{
   int id_core;
   int ejec; //Hilo que está ejecutandose
   hilo_t *hilos;
} core_t;

typedef struct{
   int id;
   core_t *cores;
} cpu_t;

typedef struct{
   int num_cpu;
   int num_core;
   int num_hilos;
   int frec_timer;
   int frec_min_pGen;
   int frec_max_pGen;
   cpu_t *cpus;
} machine_t;

extern machine_t machine;

void machine_initializer(int num_cpu, int num_core, int num_hilos, int frec_timer, int frec_min_pGen, int frec_max_pGen);
void eliminate_machine();
