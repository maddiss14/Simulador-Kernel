#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>

pthread_cond_t loader_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t loader_mutex = PTHREAD_MUTEX_INITIALIZER;

int void read_code(FILE *file)
{
   

void read_prog(char *filename, PCB *proceso)
{
   struct stat st;
   int tam_fich;
   int val_text, tam_text, pag_text;
   int val_data, tam_data, pag_data;
   int pag_tot;

   if(stat(filename, &st) == -1){
      perror("Error al intentar obtener el tamaño del fichero\n");
      exit(1);
   }

   tam_fich = st.st_size;
   if(tam_fich==0){
      perror("Tamaño del fichero es 0\n");
      exit(1);
   }

   FILE *file =fopen(dir, "rb");
   if(!file){
      perror("Error al abrir el fichero\n");
      exit(1);
   }
   
   char *buf = malloc(tam_fich);

   fread(buf, 1, tam_fich, file);
   fclose(file);

   char *linea = strtock(buf, "\n"); //Separar por lineas
   
   sscanf(linea+6, "%x", &val_text);
   printf("Valor .text %d (0x%06X)\n", val_text, val_text);

   linea=strtock(NULL, "\n");
   sscanf(linea + 6, "%x", &val_data);
   printf("Valor .data %d (0x%06X)\n", val_data, val_data);

   tam_text = val_data - val_text;
   tam_data = tam_fich-val_data;
 
   pag_text = (tam_text + TAM_PAGE-1) / TAM_PAGE;
   pag_data = (tam_data + TAM_PAGE-1) / TAM_PAGE;
   pag_tot = (tam_fic + TAM_PAGE-1)/TAM_PAGE;

   int *frames = malloc(pag_tot * sizeof(int));

   if(asig_frame_libre(frames, pag_tot)== -1){
      perror("Error al asignar los frames libres\");
      exit(1);
   }

   pcb->mm.code = val_text;
   pcb->mm.data = val_data;
   
   int aux=0;
   for(int i=0; i<pag_text; i++){
      int cont=0;
      unsigned char pal[TAM_PAL] = {0};
      while(cont<TAM_PAGE){
         for(int j=0; j<TAM_PAL; j++){
            pal[j] = buf[val_text + aux + j];
         }
         memcpy(memFisica.memoria + frames[i]*TAM_PAGE + aux, pal, TAM_PAL);
         aux +=TAM_PAL;
         cont += TAM_PAL;
      }
   }

   aux=0;
   for(int i=0; i<pag_data; i++){
      cont = 0;
      while(cont<TAM_PAGE){
         for(int j=0; j<TAM_PAL; j++){
            pal[j] = buf[val_data + aux + j];
         }
         memcpy(memFisica.memoria + frames[pag_text+i]*TAM_PAGE + aux, pal, TAM_PAL);
         aux += TAM_PAL;
         cont += TAM_PAL;
      }
   }
   aux=0;
   for(int i=pag_data; i<pag_tot; i++){
      cont=0;
      while(cont<TAM_PAGE){
         for(int j=0; j<TAM_PAL; j++){
            pal[j] = buf[val_data + tam_data + aux + j];
         }
         memcpy(memFisica.memoria + frames[pag_text+pag_data+i]*TAM_PAGE + aux, pal, TAM_PAL);
         aux += TAM_PAL;
	 cont += TAM_PAL;
      }
   }
}
