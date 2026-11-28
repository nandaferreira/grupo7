#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

#ifdef _WIN32

#include <windows.h>

typedef HANDLE pt_thread_t;
typedef CRITICAL_SECTION pt_mutex_t;

typedef struct {
    volatile LONG locked;
} pt_spinlock_t;

#elif defined(__linux__)

#include <pthread.h>

typedef pthread_t pt_thread_t;
typedef pthread_mutex_t pt_mutex_t;
typedef pthread_spinlock_t pt_spinlock_t;

#else

#error "Sistema operacional nao suportado"

#endif

// Threads
int pt_thread_create(
    pt_thread_t *thread,
    void *(*inicia_thread)(void *),
    void *arg
);

int pt_thread_join(
    pt_thread_t thread,
    void **valor_retorno
);

// Mutex
int pt_mutex_init(pt_mutex_t *mutex);
int pt_mutex_lock(pt_mutex_t *mutex);
int pt_mutex_unlock(pt_mutex_t *mutex);
int pt_mutex_destroy(pt_mutex_t *mutex);

// Spinlock
int pt_spinlock_init(pt_spinlock_t *spinlock);
int pt_spinlock_lock(pt_spinlock_t *spinlock);
int pt_spinlock_unlock(pt_spinlock_t *spinlock);
int pt_spinlock_destroy(pt_spinlock_t *spinlock);

#endif
