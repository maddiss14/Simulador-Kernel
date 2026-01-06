#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"

pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;;
int tick_timer=0;

void *timer_thread(void *arg){
   int frec_timer = machine.frec_timer;
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&timer_cond, &clock_mutex);
      pthread_cond_signal(&scheduler_cond);
      tick_timer++;
      //printf("Interrupcion timer: %d\n", tick_timer);
      pthread_mutex_unlock(&clock_mutex);
   }
   return 0;
}
