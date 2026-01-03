#include <stdio.h>
#include <stdlib.h>
#include "machine.h"

machine_t machine;

void machine_initializer(int num_cpu, int num_core, int num_hilos, int frec_timer, int frec_min_pGen, int frec_max_pGen){
   machine.num_cpu = num_cpu;
   machine.num_core = num_core;
   machine.num_hilos = num_hilos;
   machine.frec_timer = frec_timer;
   machine.frec_min_pGen = frec_min_pGen;
   machine.frec_max_pGen = frec_max_pGen;

   machine.cpus = malloc(sizeof(cpu_t) * num_cpu);
   if(machine.cpus == NULL){
      perror("Error al crear los CPUS\n");
      exit(1);
   }
   for(int i=0; i<num_cpu; i++){
      cpu_t *cpu = &machine.cpus[i];
      cpu->id = i;
      cpu->cores = malloc(sizeof(core_t) * num_core);

      for(int j=0; j<num_core; j++){
         core_t *core = &cpu->cores[j];
         core->id_core = j;
         core->hilos = malloc(sizeof(hilo_t) * num_hilos);

         for(int k=0; k<num_hilos; k++){
	    hilo_t *hilo = &core->hilos[k];
	    hilo->id_hilo = k;
	    hilo->r_pcb = NULL;
         }
      }
   }
   printf("Maquina inicializada: %d CPU, %d core, %d hilos\n", machine.num_cpu, machine.num_core, machine.num_hilos);
}

void eliminate_machine(){
int id;
   if(machine.cpus != NULL){
      for(int i=0; i<machine.num_cpu; i++){
         cpu_t *cpu = &machine.cpus[i];
         for(int j=0; j<machine.num_core; j++){
            core_t *core = &cpu->cores[j];
	    for(int k=0; k<machine.num_hilos; k++){
	       hilo_t *hilo = &core->hilos[k];
	       id = hilo->id_hilo;
	       free(hilo);
	       printf("Hilo %d liberado", id);
	    }
	    id = core->id_core;
	    free(core);
	    printf("Core %d liberado", id);
         }
         id = cpu->id;
         free(cpu);
         printf("CPU %d liberado", id);
      }
   }
}
