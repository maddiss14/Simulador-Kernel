#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"

pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;
int t_periodo=3;
int tick_timer=0;

void *timer_thread(void *arg){
   printf("Hola/n");
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&timer_cond, &clock_mutex);
      tick_timer++;
      printf("Interrupcion timer: %d/n", tick_timer);
      pthread_cond_signal(&scheduler_cond);
      if(tick_timer % t_periodo == 0){
         pthread_cond_signal(&generator_cond);
      }
      pthread_mutex_unlock(&clock_mutex);
   }
   return 0;
}
