#include <stdio.h>
#include <pthread.h>
#include <memory.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"

pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

static void ejec_hilo(hilo_t *hilo, core_t *core)
{
   if(!hilo || !hilo->r_pcb) return;
   
   if(hilo->estado == 1 && hilo->quantum>0){
      hilo->r_pcb->vida--;
      hilo->quantum--;
      printf("   Hilo ejecutando proceso %d\n", hilo->r_pcb->pid);
      
      if(hilo->quantum==0){
         if(hilo->r_pcb->vida > 0){
            add_process(hilo->r_pcb, &f_colaColas);
            printf("Hilo ejecutando proceso %d quantum acabado\n", hilo->r_pcb->pid);
         }
         else{
	    printf("Proceso %d ha terminado la ejecución\n", hilo->r_pcb->pid);
            hilo->estado=2;
         }
         hilo->r_pcb = sig_process(&r_colaColas);
	 if(hilo->r_pcb != NULL){
	    hilo->estado = 0;
	 }
         core->ejec = machine.num_hilos+1;
      }
   }else if(hilo->estado == 0){
      if( core->ejec == machine.num_hilos+1){
         core->ejec = hilo->id_hilo;
         hilo->estado = 1;
         hilo->quantum = 2;
      }
      else if(core->ejec == core->id_core){
         hilo->estado = 1;
         hilo->quantum--;
         hilo->r_pcb->vida--;
         printf("   Hilo ejecutando proceso %d\n", hilo->r_pcb->pid);
       }
   }
}
static void ejec_process()
{
   P_FCFS tmp;

   for(int i=0; i<machine.num_cpu; i++){
      cpu_t *cpu = &machine.cpus[i];
      
      for(int j=0; j<machine.num_core; j++){
         core_t *core = &cpu->cores[j];
         //printf("Ejecutando core %d\n",j);
     
         for(int k=0; k<machine.num_hilos;k++){
            hilo_t *hilo = &core->hilos[k];
      
            if(hilo->r_pcb==NULL){
               hilo->r_pcb = sig_process(&r_colaColas);
            }
            if(hilo->r_pcb != NULL){
               ejec_hilo(hilo, core);
            }
            else if(f_colaColas.num_colas != 0){
               tmp = r_colaColas;
               r_colaColas = f_colaColas;
               f_colaColas = tmp;
               
               eliminate_queue(&f_colaColas);
               politica_initializer(10, &f_colaColas);
               
               printf("Cambiando cola de preparados por cola de finalizados\n");
               hilo->r_pcb = sig_process(&r_colaColas);
	    }
         }
      }
   }
}

void *scheduler_thread(void *arg){
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&scheduler_cond, &clock_mutex);
      printf("Scheduler: %d\n", tick_timer);
      ejec_process();
      pthread_mutex_unlock(&clock_mutex);
   }
}

