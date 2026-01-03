#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"

pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;;
int tick_timer=0;
int frec_pGen;

void *timer_thread(void *arg){
   while(running){
      frec_pGen = rand() % (frecMax_pGen-frecMin_pGen) + frecMin_pGen;
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&timer_cond, &clock_mutex);
      pthread_cond_signal(&scheduler_cond);
      tick_timer++;
      printf("Interrupcion timer: %d\n", tick_timer);
      if(tick_timer % frec_pGen == 0){
         pthread_cond_signal(&generator_cond);
      }
      pthread_mutex_unlock(&clock_mutex);
   }
   return 0;
}
