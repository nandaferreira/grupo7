#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

#include <windows.h>

typedef HANDLE pt_thread_t;
typedef CRITICAL_SECTION pt_mutex_t;

int pt_thread_create(pt_thread_t *thread, void *(*inicia_thread)(void*), void *arg);
int pt_thread_join(pt_thread_t thread, void **valor_retorno);
int pt_mutex_init(pt_mutex_t *mutex);
int pt_mutex_lock(pt_mutex_t *mutex);
int pt_mutex_unlock(pt_mutex_t *mutex);
int pt_mutex_destroy(pt_mutex_t *mutex);
//Parte do Spinlock
typedef struct{
    volatile LONG locked;
}pt_spinlock_t;

int pt_spinlock_init(pt_spinlock_t *spinlock);
int pt_spinlock_lock(pt_spinlock_t *spinlock);
int pt_spinlock_unlock(pt_spinlock_t *spinlock);
int pt_spinlock_destroy(pt_spinlock_t *spinlock);

#endif

