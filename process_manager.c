#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include "process_generator.h"
#include "machine.h"

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
          hilo->r_pcb = NULL;
          hilo->estado = 2;
          core->ejec = -1;
        }
      }
    }
  }
}

void reducir_vida()
{
  for(int i=0; i<r_colaColas.num_colas; i++){
    p_queue *queue = r_colaColas.colaPrio[i];
    if(queue != NULL){
      for(int j = 0; j< queue->num_process; j++){
        if(queue->lista[j] != NULL && queue->lista[j]->vida > 0){
          queue->lista[j]->vida--;
        }
      }
    }
  }
  for(int i=0; i<f_colaColas.num_colas; i++){
    p_queue *queue = f_colaColas.colaPrio[i];
    if(queue != NULL){
      for(int j=0; j<queue->num_process; j++){
        if(queue->lista[j]->vida > 0){
          queue->lista[j]->vida--;
        }
      }
    }
  }
  
  for(int i=0; i<machine.num_cpu; i++){
    cpu_t *cpu = &machine.cpus[i];
    for(int j=0; j<machine.num_core; j++){
      core_t *core = &cpu->cores[j];
      for(int k=0; k<machine.num_hilos; k++){
        hilo_t *hilo = &core->hilos[k];
        if(hilo->r_pcb != NULL && hilo->r_pcb->vida > 0){
          hilo->r_pcb->vida--;
        }
      }
    }
  }
  elim_proc_sin_vida_colas();
  
  elim_proc_sin_vida_ejec();
}

