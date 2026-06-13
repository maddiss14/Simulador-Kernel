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
#include "process_manager.h"

pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

void queue_initializer(int tam, p_queue *queue)
{
   if(queue == NULL){
      perror("queue_initializer: queue es nulo\n");
      exit(1);
   }

   queue->lista = malloc(sizeof(PCB *)* tam);
   if(queue->lista == NULL){
      perror("Error al crear la lista de punteros \n");
      exit(1);
   }

   for (int i=0; i < tam; i++){
      queue->lista[i]=NULL;
   }

   queue->size = tam;
   queue->num_process = 0;
   queue->first = NULL;
   queue->last = NULL;
}

void eliminate_queues(P_FCFS *colaColas){
   int pid;
   int prio;

   if(colaColas->colaPrio != NULL){
      for(int i=0; i<colaColas->num_colas; i++){
         printf("Cola %d:  \n", i);

         if(colaColas->colaPrio[i]!=NULL){
            p_queue *colaProces = colaColas->colaPrio[i];

            for(int j=0; j<colaProces->num_process; j++){
               PCB *actual = colaProces->lista[j];
               pid = actual->pid;
               prio = actual->prio;

               free(actual);
               printf("   Proceso liberado %d, prioridad %d \n", pid, prio);
            }

            free(colaProces->lista);
            free(colaProces);
            printf("Cola %d liberada\n\n", i);
         }
      }
   }
}

void politica_initializer(int numPrio, P_FCFS *colaColas)
{
   colaColas->colaPrio = malloc(sizeof(p_queue *) * numPrio);
   colaColas->size = numPrio;

   if(colaColas->colaPrio == NULL){
      perror("Error al crear la cola de colas\n");
      exit(1);
   }
   colaColas->num_colas = 0;
   for(int i=0; i<colaColas->size; i++){
      colaColas->colaPrio[i] = NULL;
   }

   printf("Cola de prioridades creada e inicializada\n");
}
//Liberar memoria
void eliminate_politica(P_FCFS *colaColas)
{
   eliminate_queues(colaColas);

   free(colaColas->colaPrio);
   printf("Colas de prioridades liberadas\n");
}

static void restart_politica(P_FCFS *colaColas){
   for(int i=0; i<colaColas->num_colas; i++){
      if(colaColas->colaPrio[i]!=NULL){
         free(colaColas->colaPrio[i]->lista);
         free(colaColas->colaPrio[i]);
      }
      colaColas->colaPrio[i] = NULL;
   }
   colaColas->num_colas = 0;
   printf("Cola de prioridades reseteada \n");
}
    
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

static void elim_proc_sin_vida_colas(){
  for(int i = 0; i<r_colaColas.num_colas; i++){
    p_queue *queue = r_colaColas.colaPrio[i];
    if(queue != NULL && queue->num_process > 0){
      for(int j=0; j<queue->num_process; j++){
        if(queue->lista[j] != NULL && queue->lista[j]-> vida <= 0){
          printf("Proceso %d eliminado de la cola (sin vida)\n", queue->lista[j]->pid);
          free(queue->lista[j]);
          queue->lista[j] = NULL;
            
            //Mover array
          for(int k= j; k< queue->num_process -1; k++){
            queue->lista[k] = queue->lista[k+1];
          }
          queue->num_process--;
          queue->lista[queue->num_process] = NULL;
          
          //Actualizar punteros
          if(queue->num_process > 0){
            queue->first = queue->lista[0];
            queue->last = queue->lista[queue->num_process-1];
          }else{
            queue->first = NULL;
            queue->last = NULL; 
          }
        }
      }
    }
  }
}

static void elim_proc_sin_vida_ejec(){
  for(int i=0; i<machine.num_cpu;i++){
    cpu_t *cpu = &machine.cpus[i];
    for(int j=0; j<machine.num_core;j++){
      core_t *core = &cpu->cores[j];
      for(int k=0; k<machine.num_hilos; k++){
        hilo_t *hilo = &core->hilos[k];
        if(hilo->r_pcb != NULL && hilo->r_pcb->vida <= 0){
          printf("Proceso %d se ha quedado sin vida en hilo %d.%d.%d\n", hilo->r_pcb->pid, i, j, k);
          free(hilo->r_pcb);
          restart_hilo(hilo);
          core->ejec = -1;
        }
      }
    }
  }
}

static void asig_process(){
  for(int i=0; i<machine.num_cpu; i++){
    cpu_t *cpu = &machine.cpus[i];
    for(int j=0; j<machine.num_core; j++){
      core_t *core = &cpu->cores[j];
      for(int k=0; k<machine.num_hilos; k++){
        hilo_t *hilo = &core->hilos[k];

        if(hilo->r_pcb == NULL){
          hilo->r_pcb = sig_process(&r_colaColas);
          if(hilo->r_pcb != NULL){
            hilo->estado = 0;
            if(hilo->r_pcb->PC != -1){
              hilo->PC = hilo->r_pcb->PC;
              for(int l=0; l< NUM_REGS; l++){
                hilo->regs[l] = hilo->r_pcb->regs[l];
              }
            }else hilo->PC = hilo->r_pcb->mm.code;
            
            hilo->PTBR = hilo->r_pcb->mm.pgb;
          }
        }
      }
    }
  }
}

static void expulsar_process()
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
                  PCB *sig = sig_process(&r_colaColas);
                  if (sig != NULL){
                     cambiar_context(hilo, sig);
                     core->quantum = QUANTUM;
                     core->ejec = k;
                  }else{
                     /* No había nadie: liberar el core */
                     hilo->estado = 2;
                     core->ejec = -1;
                  }
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
      elim_proc_sin_vida_ejec();
      elim_proc_sin_vida_colas();
      asig_process();
      expulsar_process();
      pthread_mutex_unlock(&clock_mutex);
   }
}
