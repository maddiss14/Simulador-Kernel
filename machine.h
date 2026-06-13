#pragma once

#include "process_generator.h"

#define QUANTUM 2
#define TLB_ENTRIES 16
#define NUM_REGS 16

typedef struct PCB PCB;

typedef struct TLB{
   int val;  //Entrada válida
   int pid; 
   int vpn;  //Página Virtual
   int frame;//Frame físico 
} TLB;

typedef struct MMU{
   TLB tlb[TLB_ENTRIES];
} MMU;

typedef struct{
   int id_hilo;
   PCB *r_pcb;
   int estado; // 0 preparado 1 ejecutando 2 finalizado
   int PC;
   int PTBR;
   MMU mmu;
   int regs[NUM_REGS];
} hilo_t;

typedef struct{
   int id_core;
   int ejec; //Hilo que está ejecutandose
   hilo_t *hilos;
   int quantum;
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

void restart_tlb(hilo_t *hilo);
void restart_hilo(hilo_t *hilo);
void machine_initializer(int num_cpu, int num_core, int num_hilos, int frec_timer, int frec_min_pGen, int frec_max_pGen);
void eliminate_machine();
void cambiar_context(hilo_t *hilo, PCB *next_proc);
