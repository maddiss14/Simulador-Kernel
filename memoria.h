#pragma once

#include <pthread.h>

#define TAM_P_MEM (1 << 26) //64MB =  2²⁴ * 4B
#define TAM_PAL 4
#define TAM_PAGE 4096 //4KB
#define NUM_FRAMES (TAM_P_MEM/TAM_PAGE)

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
   unsigned char *memoria;
   frame_t *frames;
   int next_frame;
   int num_tables;
} physical_mem_t;

extern physical_mem_t memFisica;

extern pthread_mutex_t mem_mutex;

void phys_mem_init();
page_table_t *crear_tabla(int num_pages, const int *frames);
int asig_frame_libre(int *frames_libres, int num_page);
