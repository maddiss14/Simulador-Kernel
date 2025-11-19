#pragma once

#include <pthread.h>

extern pthread_cond_t scheduler_cond;

void *scheduler_thread(void *arg);
