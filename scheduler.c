#include <stdio.h>
#include <pthread.h>
#include <memory.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"

pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

void *scheduler_thread(void *arg){
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&scheduler_cond, &clock_mutex);
      printf("Scheduler: %d\n", tick_timer);
      pthread_mutex_unlock(&clock_mutex);
   }
}

void ejec_process(){
   for(int i=0; i<machine.num_cpu; i++){
    cpu_t *cpu = &machine.cpus[i];
    for(int j=0; j<machine.num_core; j++){
     core_t *core = &cpu->cores[j];
     for(int k=0; k<machine.num_hilos;k++){
      hilo_t *hilo = &core->hilos[k];
      if(hilo->r_pcb!=NULL){
       if(hilo->estado == 1 && hilo->quantum > 0){
        hilo->r_pcb->vida--;
        hilo->quantum--;
        if(hilo->quantum == 0){
         if(hilo->r_pcb->vida !=0){
          add_process(hilo->r_pcb, f_colaColas);
         }
         hilo->r_pcb = NULL;
         hilo->estado = 2;
         core->ejec = 3;
        }
       }
       else if(hilo->estado == 0 && core->ejec == 3){
        hilo->estado = 1;
        core->ejec = k;
        hilo->r_pcb->vida--;
        if(hilo->quantum == 0){
         if(hilo->r_pcb->vida !=0){
          add_process(hilo->r_pcb, f_colaColas);
         }
         hilo->r_pcb = NULL;
         hilo->estado = 2;
         core->ejec = 3;
        }
      }
      if(hilo->estado ==2){
        if(k != core->ejec){
         core->ejec = k;
         hilo->estado = 1;
         hilo->quantum=2;
         break;
        }
      }
    }
    else{
     hilo->r_pcb = sig_process(r_colaColas);
     if(hilo->r_pcb == NULL){
      r_colaColas = f_colaColas;
      eliminate_queue(f_colaColas);
      politica_initializer(10, f_colaColas);
     }
    }
   }
  }
 }
}      
