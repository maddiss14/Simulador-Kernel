#pragma once

#define TAM_P_MEM (1 << 26) //64MB =  2²⁴ * 4B
#define TAM_PAL 4
#define TAM_PAGE 4096 //4KB
#define NUM_FRAMES (TAM_P_MEM/TAM_PAGE)

typedef struct{
   int frame_id; //indice del frame físico
   int valida;  //1 si está mapeada
} page_t;

typedef struct{
   Pagina *pages;
   int num_pages;
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
   int numTables;
} physical_mem_t;

extern physical_mem_t memFisica;

page_table_t *crear_tabla(int num_pages, const int *frames);
