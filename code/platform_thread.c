#include "platform_thread.h"
#include <stdlib.h>
#define _WIN32

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