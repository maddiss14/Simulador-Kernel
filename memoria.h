#pragma once

#define TAM_P_MEM (1 << 26) //64MB =  2²⁴ * 4B
#define TAM_PAL 4
#define TAM_PAGE 4096 //4KB
#define NUM_FRAMES (TAM_P_MEM/TAM_PAGE)

typedef struct{
   int frame_id;
   int valida;
} page_t;

typedef struct{
   Pagina *pages;
}page_table_t;

typedef struct{
   int libre;
   int pid;
   int pagina;
}frame_t;

typedef struct{
   int *memoria;
   frame_t *frames;
   int next_frame;
   int numTables;
} physical_mem_t;

typedef struct{
   int num_pages;
   
