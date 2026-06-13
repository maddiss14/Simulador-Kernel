#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "process_generator.h"
#include "machine.h"
#include "memoria.h"

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

void ejecutar_instr(hilo_t *hilo, unsigned char instr[4])
{
  unsigned int inst = ((unsigned int)instr[0] << 24) | ((unsigned int)instr[1] << 16) | ((unsigned int)instr[2] << 8) | ((unsigned int)instr[3]);
  
  unsigned int opcode = (inst >> 28) & 0xF;
  page_table_t *tabla = memVirtual.tablas[hilo->PTBR];
  int reg = 0;
  int dir = 0;
  int fault = 0;
  switch(opcode){
  
    case 0x0: //LD
    {
      reg = (inst >> 24) & 0xF;
      dir = inst & 0xFFFFFF;
      
      unsigned char *ptr = translate_dir(hilo, dir, &fault);
      if(!ptr || fault){
        printf("Page fault en LD\n");
        break;
      }
      
      unsigned int val = ((unsigned int)ptr[0] << 24) | ((unsigned int)ptr[1] << 16) | ((unsigned int)ptr[2] << 8) | ((unsigned int)ptr[3]);
      hilo->regs[reg] = val;
      printf("LD R%d <-MEM[%d] = %u\n", reg, dir, val);
      break;
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

void ejec_hilo(){

  for(int i=0; i<machine.num_cpu; i++){
    cpu_t *cpu = &machine.cpus[i];

    for(int j=0; j<machine.num_core; j++){
      core_t *core = &cpu->cores[j];

      for(int k=0; k<machine.num_hilos; k++){
        hilo_t *hilo = &core->hilos[k];
        if(!hilo || !hilo->r_pcb) continue;
    
        //Se le ha acabado el quantum o al proceso en ejecución no le queda vida
        if(core->quantum > 0 || hilo->r_pcb->vida > 0){        
          if(hilo->estado == 1 && core->quantum>0){
            printf("   Hilo %d ejecutando proceso %d\n", hilo->id_hilo, hilo->r_pcb->pid);
            printf("      Proceso %d ptbr %d data %d\n", hilo->r_pcb->pid, hilo->r_pcb->mm.pgb, hilo->r_pcb->mm.data);
            if(memVirtual.tablas && hilo->PTBR >= 0 && hilo->PTBR < memVirtual.num_tablas && memVirtual.tablas[hilo->PTBR]){
              unsigned char instr[TAM_PAL];
              page_table_t *tabla = memVirtual.tablas[hilo->PTBR];
              int vpn = hilo->PC / TAM_PAGE;
              int offset = hilo->PC % TAM_PAGE;
                
              if(vpn >= tabla->num_pages){
                printf("Page fault: VPN %d fuera de rango\n", vpn);
                return;
              }
              int frame = tabla->pages[vpn].frame_id;
              int pa = frame * TAM_PAGE + offset;
              memcpy(instr, memFisica.memoria + pa, TAM_PAL);
              printf("INSTRUCCION PC=0x%08X PA=%d -> %02X %02X %02X %02X\n", hilo->PC, pa, instr[0], instr[1], instr[2], instr[3]);
              ejecutar_instr(hilo, instr);
              //ejecutar_instruccion(hilo, instr);
              hilo->PC +=TAM_PAL;
              core->quantum--;
            }
          }
        }
      }
    }
  }
}
      

