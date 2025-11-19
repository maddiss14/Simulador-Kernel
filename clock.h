#pragma once

#include <pthread.h>

extern pthread_mutex_t clock_mutex;
extern int tick;
extern int running;

void *clock_thread(void *arg);
void stop_clock();
void start_clock();
