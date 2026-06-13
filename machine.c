#include <stdio.h>
#include <stdlib.h>
#include "machine.h"

machine_t machine;

static void restart_tlb(hilo_t *hilo){
  for(int i = 0; i< TLB_ENTRIES; i++){
    hilo->mmu.tlb[i].val = 0;
    hilo->mmu.tlb[i].vpn = -1;
    hilo->mmu.tlb[i].frame = -1;
    hilo->mmu.tlb[i].pid = -1;
  }
}

void machine_initializer(int num_cpu, int num_core, int num_hilos, int frec_timer, int frec_min_pGen, int frec_max_pGen){
   machine.num_cpu = num_cpu;
   machine.num_core = num_core;
   machine.num_hilos = num_hilos;
   machine.frec_timer = frec_timer;
   machine.frec_min_pGen = frec_min_pGen;
   machine.frec_max_pGen = frec_max_pGen;

   machine.cpus = malloc(sizeof(cpu_t) * num_cpu);
   if(machine.cpus == NULL){
      perror("Errorr al crear los CPUS\n");
      exit(1);
   }
   for(int i=0; i<num_cpu; i++){
      cpu_t *cpu = &machine.cpus[i];
      cpu->id = i;
      cpu->cores = NULL;
      cpu->cores = malloc(sizeof(core_t) * num_core);
      if(cpu->cores == NULL){
         perror("Error al crear los cores\n");
	 exit(1);
      }
      printf("CPU %d generado\n", cpu->id);
      for(int j=0; j<num_core; j++){
         core_t *core = &cpu->cores[j];
         core->id_core = j;
	 core->hilos = NULL;
	 core->ejec = -1;
	 core->quantum = QUANTUM*frec_timer;
         core->hilos = malloc(sizeof(hilo_t) * num_hilos);
	 if(core->hilos == NULL){
	    perror("Error al crear los hilos\n");
	    exit(1);
	 }
	 printf("   Core %d generado\n", core->id_core);
         for(int k=0; k<num_hilos; k++){
	    hilo_t *hilo = &core->hilos[k];
	    hilo->id_hilo = k;
	    hilo->r_pcb = NULL;
	    hilo->estado = 2;
	    hilo->PC = 0;
	    hilo->PTBR = -1;
    
	    restart_tlb(hilo);
	    printf("      Hilo %d generado\n", hilo->id_hilo);
         }
      }
   }
   printf("Maquina inicializada: %d CPU, %d core, %d hilos\n", machine.num_cpu, machine.num_core, machine.num_hilos);
}

void eliminate_machine(){
   int id;
   
   if(machine.cpus == NULL){
      return;
   }

   for(int i=0; i<machine.num_cpu; i++){
      for(int j=0; j<machine.num_core; j++){
         free(machine.cpus[i].cores[j].hilos);
         printf("Core %d.%d, hilos liberados\n", i, j);
      }
      free(machine.cpus[i].cores);
      printf("CPU %d cores liberados\n", i);
   }
   free(machine.cpus);
   machine.num_cpu = 0;
   machine.num_core = 0;
   machine.num_hilos = 0;
   printf("Machine eliminada\n");
}
