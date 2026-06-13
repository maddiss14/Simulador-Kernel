#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "process_generator.h"
#include "machine.h"
#include "memoria.h"
#include "scheduler.h"

void ejecutar_instr(core_t *core, hilo_t *hilo, unsigned char instr[4])
{
  unsigned int inst = ((unsigned int)instr[0] << 24) | ((unsigned int)instr[1] << 16) | ((unsigned int)instr[2] << 8) | ((unsigned int)instr[3]);
  
  unsigned int opcode = (inst >> 28) & 0xF;
  page_table_t *tabla = memVirtual.tablas[hilo->PTBR];
  int reg = 0;
  int reg1 = 0;
  int reg2 = 0;
  int dir = 0;
  int fault = 0;
  switch(opcode){
  
    case 0x0: //LD
    {
      reg = (inst >> 24) & 0xF;
      dir = inst & 0x00FFFFFF;
      
      unsigned char *ptr = translate_dir(hilo, dir, &fault);
      if(!ptr || fault){
        printf("Page fault en LD\n");
        break;
      }
      
      unsigned int val = ((unsigned int)ptr[0] << 24) | ((unsigned int)ptr[1] << 16) | ((unsigned int)ptr[2] << 8) | ((unsigned int)ptr[3]);
      hilo->regs[reg] = val;
      printf("LD R%d <-MEM[%d] = %d\n", reg, dir, val);
      break;
    }
    
    case 0x1:
    {
      reg = (inst >> 24) & 0xF;
      dir = inst & 0x00FFFFFF;
      
      unsigned char *ptr = translate_dir(hilo, dir, &fault);
      if(!ptr || fault){
        printf("Page fault en LD\n");
        break;
      }
      unsigned int val = hilo->regs[reg];
      ptr[0] = (val >> 24) & 0xFF;
      ptr[1] = (val >> 16) & 0xFF;
      ptr[2] = (val >> 8) & 0xFF;
      ptr[3] = (val) & 0xFF;
      
      printf("ST MEM[%d] <- R%d = %d\n", reg, dir, val);
      break;
    }
    case 0x2:
    {
      reg = (inst >> 24) & 0xF;
      reg1 = (inst >> 20) & 0xF;
      reg2 = (inst >> 16) & 0xF;
      hilo->regs[reg] = hilo->regs[reg1] + hilo->regs[reg2];
      printf("ADD R%d = R%d + R%d = %d\n", reg, reg1, reg2, hilo->regs[reg]);
      break;
    }
    case 0xF:
    {
      printf("\n   EL PROCESO %d HA TERMINADO SU EJECUCIÓN\n", hilo->r_pcb->pid);
      
      PCB *p = hilo->r_pcb;
      p->vida = -1;
      hilo->r_pcb = NULL;
      hilo->estado = 2;
      core->ejec = -1;
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
}

void ejec_hilo(){

  for(int i=0; i<machine.num_cpu; i++){
    cpu_t *cpu = &machine.cpus[i];

    for(int j=0; j<machine.num_core; j++){
      core_t *core = &cpu->cores[j];

      for(int k=0; k<machine.num_hilos; k++){
        hilo_t *hilo = &core->hilos[k];
        if(!hilo || !hilo->r_pcb) continue;

        if(core->quantum > 0 && hilo->r_pcb->vida > 0){        
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
              ejecutar_instr(core, hilo, instr);
              //ejecutar_instruccion(hilo, instr);
              hilo->PC +=TAM_PAL;
              core->quantum--;
            }
          }else if(core->quantum <= 0)
          {
            printf("QUANTUM DE %d se ha acabado\n", hilo->r_pcb->pid);
            int hay=0;
            for(int l=0; l<machine.num_hilos; l++){
              if(hilo != &core->hilos[l]){
                if(core->hilos[l].estado == 0 && core->hilos[l].r_pcb != NULL)
                {
                  add_process(hilo->r_pcb, &f_colaColas);
                  PCB *sig = core->hilos[l].r_pcb;
                  cambiar_context(hilo, sig);
                  core->quantum = QUANTUM;
                  core->ejec = l;
                  hay = 1;
                }
              }
            }
            if(!hay){
              hilo->r_pcb->PC = hilo->PC;
              for(int i=0; i<NUM_REGS; i++){
                hilo->r_pcb->regs[i] = hilo->regs[i];
              }
              core->ejec = -1;
              hilo->estado = 2;
              core->quantum = QUANTUM;
              restart_tlb(hilo);
              add_process(hilo->r_pcb, &f_colaColas);
              hilo->r_pcb = NULL;
            }
          }
        }
      }
    }
  }
}
      

