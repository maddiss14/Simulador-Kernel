#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"

int main(void *arg){
   pthread_t clock_t, timer_t, scheduler_t, generator_t;

   printf("Inicio simulacion \n");
   start_clock();

   pthread_create(&clock_t, NULL, clock_thread, NULL);
   pthread_create(&timer_t, NULL, timer_thread, NULL);
   pthread_create(&scheduler_t, NULL, scheduler_thread, NULL);
   pthread_create(&generator_t, NULL, generator_thread, NULL);

   queue_initializer(5);

   sleep(20);

   stop_clock();

   eliminate_queue();

   sleep(1);
   
   pthread_cond_broadcast(&generator_cond);
   pthread_cond_broadcast(&timer_cond);
   pthread_cond_broadcast(&scheduler_cond);


   pthread_join(clock_t, NULL);
   pthread_join(generator_t, NULL);
   pthread_join(timer_t, NULL);
   pthread_join(scheduler_t, NULL);

   printf("Simulacion terminada /n");
   return 0;
}
