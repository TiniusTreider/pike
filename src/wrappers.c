#define _POSIX_C_SOURCE 200809L

#include "wrappers.h"
#include "error.h"

#include <errno.h>
#include <time.h>

void init_thread(p_thread *restrict thread, void *(*function)(void*), void *restrict arg)
{
        if (pthread_create(thread, NULL, function, arg))
                error("failed to initialize thread");
}

void clean_thread(p_thread *thread) {
        if (pthread_join(*thread, NULL))
                error("failed to clean thread");
}

void init_mutex(p_mutex *mutex) {
        if (pthread_mutex_init(mutex, NULL))
                error("failed to initialize mutex");
}

void clean_mutex(p_mutex *mutex) {
        if (pthread_mutex_destroy(mutex))
                error("failed to destroy mutex");
}

void lock_mutex(p_mutex *mutex) {
        if (pthread_mutex_lock(mutex))
                error("failed to lock mutex");
}

void unlock_mutex(p_mutex *mutex) {
        if (pthread_mutex_unlock(mutex))
                error("failed to unlock mutex");
}

void init_sem(p_sem *sem) {
        if (sem_init(sem, 0, 0))
                error("failed to initialize semaphore");
}

void clean_sem(p_sem *sem) {
        if (sem_destroy(sem))
                error("failed to destroy semaphore");
}

void post_sem(p_sem *sem) {
        if (sem_post(sem))
                error("failed to perform semaphore post");
}

void wait_sem(p_sem *sem) {
        while (sem_wait(sem))
        {
                if (errno != EINTR)
                        error("failed to perform semaphore wait");
        }
}

#define MS_IN_S 1000
#define NS_IN_MS 1000000

size_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * MS_IN_S + ts.tv_nsec / NS_IN_MS;
}

