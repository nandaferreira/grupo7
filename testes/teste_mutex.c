#include "../code/platform_thread.h"
#include <stdio.h>
#define THREADS_TRABALHO 4
#define INCREMENTS_PER_THREAD 100000
#include <windows.h>

static int contador_compartilhado = 0;
static pt_mutex_t mutex_contador;

void *task_incremento(void *arg){
    (void)arg;

    for(int i = 0; i < INCREMENTS_PER_THREAD; i++){
        pt_mutex_lock(&mutex_contador);
        contador_compartilhado++; //seção crítica
        pt_mutex_unlock(&mutex_contador);
    }
    return NULL;
}

int main(void){
    pt_thread_t threads[THREADS_TRABALHO];
    printf(" teste da camada de abstracao: mutex\n");
    printf("========================================\n\n");

    if (pt_mutex_init(&mutex_contador) != 0){
        fprintf(stderr, "erro: mutex nao inicializado\n");
        return 1;
    }
    printf("ok: mutex inicializado.\n");
    Sleep(1000);

    for(int i = 0; i < THREADS_TRABALHO; i++){
        if(pt_thread_create(&threads[i], task_incremento, NULL) != 0){
            fprintf(stderr, "erro: nao foi possivel criar a thread %d.\n", i + 1);
            return 1;
        }
    }
    printf("ok: Threads criadas: %d.\n", THREADS_TRABALHO);

    for(int i = 0; i < THREADS_TRABALHO; i ++){
        if(pt_thread_join(threads[i], NULL) != 0){
            fprintf(stderr, "erro: nao foi possivel aguardar a thread %d\n", i + 1);
            return 1;
        }
        printf("  - thread %d finalizada.\n", i + 1);
    }
    printf("ok: todas as threads foram finalizadas\n");

    if(pt_mutex_destroy(&mutex_contador) != 0){
        fprintf(stderr, "erro: nao foi possivel destruir o mutex\n");
        return 1;
    }
    printf("ok: mutex destruido.\n\n");

    int esperado = THREADS_TRABALHO * INCREMENTS_PER_THREAD;
    printf("resultado esperado: %d\n", esperado);
    printf("resultado obtido : %d\n", contador_compartilhado);

    if(contador_compartilhado == esperado){
        printf("\nresultado: sucesso - os valores correspondem.\n");
        return 0;
    }else{
        printf("\nresultado: falha - os valores sao diferentes.\n");
        return 1;
    }

}