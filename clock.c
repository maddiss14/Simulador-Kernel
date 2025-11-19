#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "clock.h"
#include "timer.h"
#include "scheduler.h"

pthread_mutex_t clock_mutex = PTHREAD_MUTEX_INITIALIZER;
int tick=0;
int running;


void *clock_thread(void *arg){
   while(running){
      sleep(1);
      pthread_mutex_lock(&clock_mutex);
      tick++;
      printf("Clock tick: %d\n", tick);
      if(tick % t_periodo == 0){
         pthread_cond_signal(&timer_cond);
         printf("timer cond");
      }
      pthread_mutex_unlock(&clock_mutex);
      printf("Unlocked \n");
   }
   return 0;
}
void start_clock(){
   running=1;
}

void stop_clock(){
   running=0;
}
