#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"


int main(int argc, char *argv[]){
   pthread_t clock_t, timer_t, scheduler_t, generator_t;

   int num_cpu, num_core, num_hilos, frec_timer, frecMin_pGen, frecMax_pGen;

   if(argc != 7){
	printf("Uso: %s num_cpus, num_cores, num_hilos, frec_timer frecMin_pGenerator frecMax_pGenerator \n", argv[0]);
	return(1);
   }
   
   srand(time(NULL));

   num_cpu = atoi(argv[1]);
   num_core = atoi(argv[2]);
   num_hilos = atoi(argv[3]);
   frec_timer = atoi(argv[4]);
   frecMin_pGen = atoi(argv[5]);
   frecMax_pGen = atoi(argv[6]);

   machine_initializer(num_cpu, num_core, num_hilos, frec_timer, frecMin_pGen, frecMax_pGen);
   printf("Inicio simulacion \n");
   start_clock();

   pthread_create(&clock_t, NULL, clock_thread, NULL);
   pthread_create(&timer_t, NULL, timer_thread, NULL);
   pthread_create(&scheduler_t, NULL, scheduler_thread, NULL);
   pthread_create(&generator_t, NULL, generator_thread, NULL);

   politica_initializer(10);

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
   eliminate_machine();
   printf("Simulacion terminada \n");
   return 0;
}
