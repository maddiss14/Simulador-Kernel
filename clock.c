#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "clock.h"
#include "timer.h"
#include "scheduler.h"
#include "machine.h"
#include "process_generator.h"
#include "process_manager.h"

pthread_mutex_t clock_mutex = PTHREAD_MUTEX_INITIALIZER;
int tick=0;
int running;
int frec_pGen;

void *clock_thread(void *arg){
   int frec_timer = machine.frec_timer;
   int frecMax_pGen = machine.frec_max_pGen;
   int frecMin_pGen = machine.frec_min_pGen;
   
   while(running){
      frec_pGen = rand() % (frecMax_pGen-frecMin_pGen) + frecMin_pGen;
      sleep(1);
      pthread_mutex_lock(&clock_mutex);
      reducir_vida();
      tick++;
      //printf("Clock tick: %d\n", tick);
      if(tick % frec_timer == 0){
         pthread_cond_signal(&timer_cond);
      }
      if(tick % frec_pGen == 0){
         pthread_cond_signal(&generator_cond);
      }
      pthread_mutex_unlock(&clock_mutex);
      //printf("Unlocked \n");
   }
   return 0;
}
void start_clock(){
   running=1;
}

void stop_clock(){
   running=0;
}
