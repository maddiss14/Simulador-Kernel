#pragma once

#include <pthread.h>

extern pthread_cond_t timer_cond;
extern int t_periodo;
extern int tick_timer;

void *timer_thread(void *arg);
