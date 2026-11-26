#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define REPETICOES 100000

long contador = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t spinlock;

int usar_spinlock = 0;

void verificar(int erro) {
    if (erro != 0) {
        fprintf(stderr, "Erro pthread: %d\n", erro);
        exit(EXIT_FAILURE);
    }
}

void *executar(void *arg) {
    (void)arg;

    for (int i = 0; i < REPETICOES; i++) {

        // Regiao de entrada
        if (usar_spinlock) {
            verificar(pthread_spin_lock(&spinlock));
        } else {
            verificar(pthread_mutex_lock(&mutex));
        }

        // Regiao critica
        contador++;

        // Regiao de saida
        if (usar_spinlock) {
            verificar(pthread_spin_unlock(&spinlock));
        } else {
            verificar(pthread_mutex_unlock(&mutex));
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 2 ||
        (strcmp(argv[1], "mutex") != 0 &&
         strcmp(argv[1], "spin") != 0)) {
        printf("Uso: %s mutex|spin\n", argv[0]);
        return 1;
    }

    usar_spinlock = strcmp(argv[1], "spin") == 0;

    pthread_t threads[NUM_THREADS];

    if (usar_spinlock) {
        verificar(pthread_spin_init(
            &spinlock, PTHREAD_PROCESS_PRIVATE
        ));
    }

    // Criacao das threads
    for (int i = 0; i < NUM_THREADS; i++) {
        verificar(pthread_create(
            &threads[i], NULL, executar, NULL
        ));
    }

    // Aguarda a finalizacao das threads
    for (int i = 0; i < NUM_THREADS; i++) {
        verificar(pthread_join(threads[i], NULL));
    }

    printf("\n===== TESTE DE SINCRONIZACAO =====\n");
    printf("Sistema operacional: Linux\n");
    printf("Mecanismo: %s\n", argv[1]);
    printf("Numero de threads: %d\n", NUM_THREADS);
    printf("Incrementos por thread: %d\n", REPETICOES);
    printf("Contador final: %ld\n", contador);
    printf("Valor esperado: %d\n",
           NUM_THREADS * REPETICOES);

    if (contador == NUM_THREADS * REPETICOES) {
        printf("Resultado: SUCESSO!\n");
    } else {
        printf("Resultado: FALHA!\n");
    }

    if (usar_spinlock) {
        verificar(pthread_spin_destroy(&spinlock));
    }

    verificar(pthread_mutex_destroy(&mutex));

    return contador != NUM_THREADS * REPETICOES;
}
