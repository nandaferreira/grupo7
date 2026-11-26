#include "platform_thread.h"
#include <stdlib.h>
#include <stdio.h>

int pt_thread_create(pt_thread_t *thread, void *(*inicia_thread)(void*), void *arg){
    if(thread == NULL || inicia_thread == NULL){
        printf("erro na criacao da thread\n");
        return -1;
    }

    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) inicia_thread, arg, 0, NULL);
    if(*thread == NULL){
        printf("erro na criacao da thread\n");
        return -1;
    }
    return 0;
}

int pt_thread_join(pt_thread_t thread, void **valor_retorno){
    (void)valor_retorno;
    DWORD resultado = WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return(resultado == WAIT_OBJECT_0) ? 0 : -1;
}

int pt_mutex_init(pt_mutex_t *mutex){
    if(mutex == NULL){return -1;}
    InitializeCriticalSection(mutex);
    return 0;
}

int pt_mutex_lock(pt_mutex_t *mutex){
    if(mutex == NULL){return -1;}
    EnterCriticalSection(mutex);
    return 0;
}

int pt_mutex_unlock(pt_mutex_t *mutex){
    if(mutex == NULL){return -1;}
    LeaveCriticalSection(mutex);
    return 0;
}

int pt_mutex_destroy(pt_mutex_t *mutex){
    if(mutex == NULL){return -1;}
    DeleteCriticalSection(mutex);
    return 0;
}

//Parte do Spinlock
int pt_spinlock_init(pt_spinlock_t *spinlock){
    if(spinlock == NULL){return -1;}
    spinlock->locked = 0;
    return 0;
}

int pt_spinlock_lock(pt_spinlock_t *spinlock){
    if(spinlock == NULL){return -1;}
    while(InterlockedExchange(&spinlock->locked, 1) != 0){
        while(spinlock->locked != 0){
            YieldProcessor();
        }
    }
    return 0;
}

int pt_spinlock_unlock(pt_spinlock_t *spinlock){
    if(spinlock == NULL){return -1;}
    InterlockedExchange(&spinlock->locked, 0);
    return 0;
}

int pt_spinlock_destroy(pt_spinlock_t *spinlock){
    if(spinlock == NULL){return -1;}
    spinlock->locked = 0;
    return 0;
}
