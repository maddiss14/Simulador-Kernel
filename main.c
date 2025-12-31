#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"

int frec_timer;
int frecMin_pGen;
int frecMax_pGen;

int main(int argc, char *argv[]){
   pthread_t clock_t, timer_t, scheduler_t, generator_t;

   if(argc != 4){
	printf("Uso: %s frec_timer frecMin_pGenerator frecMax_pGenerator \n", argv[0]);
	return(1);
   }
   frec_timer = atoi(argv[1]);
   frecMin_pGen = atoi(argv[2]);
   frecMax_pGen = atoi(argv[3]);
   printf("Inicio simulacion \n");
   start_clock();

   pthread_create(&clock_t, NULL, clock_thread, NULL);
   pthread_create(&timer_t, NULL, timer_thread, NULL);
   pthread_create(&scheduler_t, NULL, scheduler_thread, NULL);
   pthread_create(&generator_t, NULL, generator_thread, NULL);

   politica_initializer(5);

   sleep(20);

   stop_clock();

   //Despertar procesos
   pthread_cond_broadcast(&timer_cond);
   pthread_cond_broadcast(&scheduler_cond);
   pthread_cond_broadcast(&generator_cond);
//   pthread_join(generator_t, NULL);
//   pthread_join(scheduler_t, NULL);
//   pthread_join(timer_t, NULL);
   pthread_join(clock_t, NULL);
   pthread_join(generator_t, NULL);
   pthread_join(timer_t, NULL);
   pthread_join(scheduler_t, NULL);

   eliminate_queue();

   printf("Simulacion terminada \n");
   return 0;
}
