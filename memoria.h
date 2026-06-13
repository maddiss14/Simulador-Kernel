#pragma once

#include <pthread.h>
#include "machine.h"

#define TAM_P_MEM (1 << 26) //64MB =  2²⁴ * 4B
#define TAM_PAL 4
#define TAM_PAGE 128 //4KB
#define NUM_FRAMES (TAM_P_MEM/TAM_PAGE)
#define KERNEL_FRAMES 128

typedef struct{
   int frame_id; //indice del frame físico
   int valida;  //1 si está mapeada
} page_t;

typedef struct{
   page_t *pages;
   int num_pages;
   int id;
}page_table_t;

typedef struct{
   int libre;
   int pid;
   int pagina; //id pag proceso
}frame_t;

typedef struct{
   page_table_t **tablas;
   int tam;
   int num_tablas;
} virtual_mem_t;

typedef struct{
   unsigned char *memoria;
   frame_t *frames;
   int next_frame;
   int num_tables;
   int kernel_next;
} physical_mem_t;

extern physical_mem_t memFisica;
extern virtual_mem_t memVirtual;

extern pthread_mutex_t mem_mutex;

int tlb_acc(hilo_t *hilo, int pag_num);
int add_tlb(hilo_t *hilo, int pag_num, int marco);
void phys_mem_init();
void virt_mem_init();
page_table_t *crear_tabla(int num_pages, const int *frames, int pid);
int asig_frame_libre(int *frames_libres, int num_page);
void mm_read(hilo_t *hilo, int va, char *out, int *fault);
int add_ptable(page_table_t *tabla);
unsigned char *translate_dir(hilo_t *hilo,int va, int *page_fault);
void printf_tablaPag(page_table_t *tabla);
