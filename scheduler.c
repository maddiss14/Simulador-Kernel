#include <stdio.h>
#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"
#include "memoria.h"

pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

//Comprueba si hay un proceso preparado con mayor prioridad
static int hay_mayor_prioridad(hilo_t *hilo) {
    if (hilo->r_pcb==NULL) return 0;

    if(r_colaColas.num_colas != 0){
       for(int i=0; i<r_colaColas.num_colas; i++){
          p_queue *queue = r_colaColas.colaPrio[i];
          if(queue && queue->num_process != 0){
             PCB *proceso = queue->first;
             PCB *act = hilo->r_pcb;
             if (proceso && proceso->prio < act->prio) return 1;
          }
       }
    }
    return 0;
}

//Devuelve 1 si hay procesos preparados
static int hay_preparados_en_colas()
{
   if (r_colaColas.num_colas == 0 || r_colaColas.colaPrio == NULL) return 0;
   for (int i=0; i<r_colaColas.num_colas; i++){
      p_queue *q = r_colaColas.colaPrio[i];
      if (q != NULL && q->num_process > 0) return 1;
   }
   return 0;
}

static void ejec_hilo(hilo_t *hilo, core_t *core)
{
   if(!hilo || !hilo->r_pcb) return;
  
   //Se le ha acabado el quantum o al proceso en ejecución no le queda vida
   if(core->quantum == 0 || hilo->r_pcb->vida == 0){

      if(hilo->r_pcb->vida > 0){
         //Política expulsora por quantum
         if(hay_preparados_en_colas()){
            printf("   Hilo %d cede CPU: proceso %d quantum acabado\n",
                   hilo->id_hilo, hilo->r_pcb->pid);

            add_process(hilo->r_pcb, &f_colaColas);

            hilo->r_pcb = NULL;
            hilo->estado = 2;      /* sin proceso */
            core->ejec = -1;       /* libera el core */
         }else{
            core->quantum = QUANTUM;
         }
      }else{
         printf("   Proceso %d terminado en hilo %d\n", hilo->r_pcb->pid, hilo->id_hilo);
         hilo->r_pcb = NULL;
         hilo->estado = 2;
         core->ejec = -1;
      }
   }

   if(hilo->estado == 1 && core->quantum>0){
      printf("   Hilo %d ejecutando proceso %d\n", hilo->id_hilo, hilo->r_pcb->pid);
      printf("      Proceso %d ptbr %d data %d\n", hilo->r_pcb->pid, hilo->r_pcb->mm.pgb, hilo->r_pcb->mm.data);
      if(memVirtual.tablas && hilo->PTBR >= 0 && hilo->PTBR < memVirtual.num_tablas && memVirtual.tablas[hilo->PTBR]){
      
        int fallo = 0;
        unsigned char instr[TAM_PAL] = {0};
        printf_tablaPag(memVirtual.tablas[hilo->r_pcb->mm.pgb]);
        int frame = memVirtual.tablas[hilo->r_pcb->mm.pgb]->pages[0].frame_id;
        int base = frame*TAM_PAGE;
         printf("LOLOLO %02X %02X %02X %02X\n", memFisica.memoria[base], memFisica.memoria[base+1], memFisica.memoria[base+2], memFisica.memoria[base+3]);
         mm_read(hilo, hilo->PC, instr, &fallo);
         if(fallo){
            printf("   Hilo %d ejecutando proceso %d: en VA=0x%08X\n", hilo->id_hilo, hilo->r_pcb->pid, hilo->PC);
         }else{
            printf("INSTRUCCION PC=0x%08X INSTR =%02X %02X %02X %02X\n", hilo->PC, instr[0], instr[1], instr[2], instr[3]);
            hilo->PC +=TAM_PAL;
         }
         core->quantum--;
      }
   }
}

static void asig_process(){
   for(int i=0; i<machine.num_cpu; i++){
      cpu_t *cpu = &machine.cpus[i];
      for(int j=0; j<machine.num_core; j++){
         core_t *core = &cpu->cores[j];


         if(core->ejec == -1){
            for(int k=0; k<machine.num_hilos; k++){
               hilo_t *hilo = &core->hilos[k];

               if(hilo->r_pcb == NULL){
                  hilo->r_pcb = sig_process(&r_colaColas);

                  if(hilo->r_pcb != NULL){
                     hilo->PTBR = hilo->r_pcb->mm.pgb;
                     hilo->PC = hilo->r_pcb->mm.code;
                     hilo->estado = 1;
                     core->quantum = QUANTUM;
                     core->ejec = hilo->id_hilo;
                     break;
                  }
               }
            }
         }
      }
   }
}

static void ejec_process()
{

   for(int i=0; i<machine.num_cpu; i++){
      cpu_t *cpu = &machine.cpus[i];

      for(int j=0; j<machine.num_core; j++){
         core_t *core = &cpu->cores[j];

         for(int k=0; k<machine.num_hilos; k++){
            hilo_t *hilo = &core->hilos[k];

            if(hilo->estado == 0 && hilo->r_pcb != NULL && core->ejec == -1){
               hilo->estado = 1;
               core->quantum = QUANTUM;
               core->ejec = k;
            }

            if (hilo->estado == 1 && hilo->r_pcb != NULL) {
               if (hay_mayor_prioridad(hilo)) {
                  printf(" Expulsando proceso %d por llegada de uno con mayor prioridad\n",
                         hilo->r_pcb->pid);

                  add_process(hilo->r_pcb, &f_colaColas);
                  hilo->r_pcb = sig_process(&r_colaColas);

                  if (hilo->r_pcb != NULL){
                     hilo->estado = 1;
                     core->quantum = QUANTUM;
                     core->ejec = k;
                     ejec_hilo(hilo, core);
                  }else{
                     /* No había nadie: liberar el core */
                     hilo->estado = 2;
                     core->ejec = -1;
                  }
               } else {
                  ejec_hilo(hilo, core);
               }
            }
         }

         if(core->ejec == -1 && r_colaColas.num_colas == 0 && f_colaColas.num_colas != 0){
            P_FCFS tmp = r_colaColas;
            r_colaColas = f_colaColas;
            f_colaColas = tmp;
            restart_politica(&f_colaColas);
            printf("Cambiando cola de preparados por cola de finalizados\n");
         }
      }
   }
}


void *scheduler_thread(void *arg){
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&scheduler_cond, &clock_mutex);
      printf("Scheduler: %d\n", tick_timer);
      asig_process();
      ejec_process();
      pthread_mutex_unlock(&clock_mutex);
   }
}
