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


