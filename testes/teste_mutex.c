#include "../code/platform_thread.h"
#include <stdio.h>
#define THREADS_TRABALHO 4
#define INCREMENTS_PER_THREAD 10000

static int contador_compartilhado = 0;
static pt_mutex_t mutex_contador;

void *task_incremento(void *arg){
    int id = *(int *)arg;
    printf("thread %d iniciada\n", id);

    for(int i = 0; i < INCREMENTS_PER_THREAD; i++){
        pt_mutex_lock_(&mutex_contador);
        contador_compartilhado++; //seção crítica
        pt_mutex_unlock(&mutex_contador);
    }
    printf("thread finalizada\n", id);
    return NULL;
}

int main(void){
    pt_thread_t threads[THREADS_TRABALHO];
    int ids[THREADS_TRABALHO];

    printf("--teste mutex --\n");
    //1) teste iniciar

    if (pt_mutex_init(&mutex_contador) != 0){
        fprintf(stderr, "erro de inicializacao do mutex\n");
        return 1;
    }
    
    printf("mutex inicializado\n");

    //2) teste criaacao
    for(int i = 0; i < THREADS_TRABALHO; i++){
        ids[i] = i + 1;
        if(pt_thread_create(&threads[i], task_incremento, &ids[i]) != 0){
            fprintf(stderr, "erro ao criar a thread %d \n", i +1);
            return 1;
        }
    }
    printf("%d threads criadas", THREADS_TRABALHO);

    //3) teste espera
    for(int i = 0; i < THREADS_TRABALHO; i ++){
        if(pt_thread_join(threads[i], NULL) != 0){
            fprintf(stderr, "erro na juncao da thread %d\n", i +1);
            return 1;
        }
    }
    printf("threads finalizadas\n");

    //4) teste destruir mutex
    if(pt_mutex_destroy(&mutex_contador) != 0){
        fprintf(stderr, "erro destruicao mutex\n");
        return 1;
    }
    printf("thread excluida\n");

    //5) teste mutex

    int esperado = THREADS_TRABALHO * INCREMENTS_PER_THREAD;
    printf("\nresultado esperado: %d\n", esperado);
    printf("resultado obtido: %d \n", contador_compartilhado);

    if(contador_compartilhado == esperado){
        printf("nenhuma condicao de corrida encontrada \n");
        return 0;
    }else{
        printf("falha - condicao de corrida encontrada\n");
        return 1;
    }

}