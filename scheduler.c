#include <stdio.h>
#include <pthread.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "process_generator.h"
#include "machine.h"

pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

void *scheduler_thread(void *arg){
   while(running){
      pthread_mutex_lock(&clock_mutex);
      pthread_cond_wait(&scheduler_cond, &clock_mutex);
      printf("Scheduler: %d\n", tick_timer);
      pthread_mutex_unlock(&clock_mutex);
   }
}

