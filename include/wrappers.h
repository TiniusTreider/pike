#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <pthread.h>
#include <semaphore.h>

typedef pthread_t p_thread;
typedef pthread_mutex_t p_mutex;
typedef sem_t p_sem;

void init_thread(p_thread *restrict thread, void *(*function)(void*), void *restrict arg);
void clean_thread(p_thread *thread);

void init_mutex(p_mutex *mutex);
void clean_mutex(p_mutex *mutex);
void lock_mutex(p_mutex *mutex);
void unlock_mutex(p_mutex *mutex);

void init_sem(p_sem *sem);
void clean_sem(p_sem *sem);
void post_sem(p_sem *sem);
void wait_sem(p_sem *sem);

size_t now_ms(void);

#endif

