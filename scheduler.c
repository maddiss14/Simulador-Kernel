#include <stdio.h>
#include <pthread.h>
#include <memory.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"

pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

static int procesos_preparados(core_t *core, hilo_t *hilo){
   for (int i=0; i<machine.num_core; i++){
      hilo_t *act = &core->hilos[i];
      if(act->r_pcb != NULL && act != hilo) return 1;
   }
   return 0;
}

static void ejec_hilo(hilo_t *hilo, core_t *core)
{
   if(!hilo || !hilo->r_pcb) return;
   
   if(hilo->estado == 1 && hilo->quantum>0){
      hilo->r_pcb->vida--;
      hilo->quantum--;

      printf("   Hilo %d ejecutando proceso %d\n",hilo->id_hilo, hilo->r_pcb->pid);
      
      if(hilo->quantum==0){
      
         if(hilo->r_pcb->vida > 0){
               
            if(!procesos_preparados(core, hilo)){
               hilo->quantum = 2;
               core->ejec = hilo->id_hilo;
               return;
            }
            printf("Hilo ejecutando proceso %d quantum acabado\n", hilo->r_pcb->pid);
            add_process(hilo->r_pcb, &f_colaColas);
            hilo->r_pcb = NULL;
            hilo->estado = 0;
         
         }else{
	    printf("Proceso %d ha terminado la ejecución\n", hilo->r_pcb->pid);
            hilo->estado=2;
            hilo->r_pcb = NULL;
         }
         core->ejec = -1;
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
	 core->ejec = -1;
         //printf("Ejecutando core %d\n",j);
     
         for(int k=0; k<machine.num_hilos;k++){
            hilo_t *hilo = &core->hilos[k];
      
            if(core->ejec != -1) break;

            if(hilo->r_pcb==NULL){
               hilo->r_pcb = sig_process(&r_colaColas);
            
	       if(hilo->r_pcb != NULL){
                  hilo->estado = 0;
               }
	    }

	    if(hilo->estado == 0 && core->ejec == -1){
   	       hilo->estado = 1;
	       hilo->quantum = 2;
	       core->ejec = k;
       	    }

            if(hilo->estado == 1){
	       ejec_hilo(hilo, core);
               break;
	    }

            if(r_colaColas.num_colas == 0 && f_colaColas.num_colas != 0){
               r_colaColas = f_colaColas;
               politica_initializer(10, &f_colaColas);
               
               printf("Cambiando cola de preparados por cola de finalizados\n");
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

