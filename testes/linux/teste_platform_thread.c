#include "../../code/platform_thread.h"
#include <stdio.h>
#include <stdlib.h>

#define THREADS 4
#define INCREMENTOS 100000

static long contador;
static pt_mutex_t mutex;
static pt_spinlock_t spinlock;

static void verificar(int resultado) {
    if (resultado != 0) {
        fprintf(stderr, "Erro: %d\n", resultado);
        exit(EXIT_FAILURE);
    }
}

static void *teste_mutex(void *arg) {
    (void)arg;

    for (int i = 0; i < INCREMENTOS; i++) {
        verificar(pt_mutex_lock(&mutex));
        contador++;
        verificar(pt_mutex_unlock(&mutex));
    }

    return NULL;
}

static void *teste_spinlock(void *arg) {
    (void)arg;

    for (int i = 0; i < INCREMENTOS; i++) {
        verificar(pt_spinlock_lock(&spinlock));
        contador++;
        verificar(pt_spinlock_unlock(&spinlock));
    }

    return NULL;
}

int main(void) {
    pt_thread_t threads[THREADS];

    const long esperado = THREADS * INCREMENTOS;

    contador = 0;

    verificar(pt_mutex_init(&mutex));

    for (int i = 0; i < THREADS; i++)
        verificar(pt_thread_create(
            &threads[i], teste_mutex, NULL));

    for (int i = 0; i < THREADS; i++)
        verificar(pt_thread_join(threads[i], NULL));

    verificar(pt_mutex_destroy(&mutex));

    printf("Mutex: %ld | Esperado: %ld | %s\n",
           contador, esperado,
           contador == esperado ? "OK" : "ERRO");

    if (contador != esperado)
        return EXIT_FAILURE;

    contador = 0;

    verificar(pt_spinlock_init(&spinlock));

    for (int i = 0; i < THREADS; i++)
        verificar(pt_thread_create(
            &threads[i], teste_spinlock, NULL));

    for (int i = 0; i < THREADS; i++)
        verificar(pt_thread_join(threads[i], NULL));

    verificar(pt_spinlock_destroy(&spinlock));

    printf("Spinlock: %ld | Esperado: %ld | %s\n",
           contador, esperado,
           contador == esperado ? "OK" : "ERRO");

    return contador == esperado
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}
