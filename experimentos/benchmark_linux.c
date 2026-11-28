#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include "../code/platform_thread.h"
#include <time.h>
#include <stdint.h>

#define TOTAL_AQUISICOES 16000
#define REPETICOES 30
#define MAX_THREADS 16

static pt_mutex_t mutex;
static pt_spinlock_t spinlock;
static long contador;

typedef struct {
    int aquisicoes;
    int duracao_us;
    int usar_spinlock;
} tarefa_t;

static void verificar(int erro) {
    if (erro != 0) {
        fprintf(stderr, "Erro pthread: %d\n", erro);
        exit(EXIT_FAILURE);
    }
}

static uint64_t agora_ns(void) {
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (uint64_t)ts.tv_sec * 1000000000ULL
           + (uint64_t)ts.tv_nsec;
}

// Simula trabalho dentro da regiao critica
static void simular_trabalho(int microssegundos) {
    if (microssegundos <= 0)
        return;

    uint64_t inicio = agora_ns();
    uint64_t duracao = (uint64_t)microssegundos * 1000;

    while (agora_ns() - inicio < duracao) {
        // Trabalho simulado
    }
}

// Funcao executada por cada thread
static void *executar(void *arg) {
    tarefa_t *tarefa = arg;

    for (int i = 0; i < tarefa->aquisicoes; i++) {

        if (tarefa->usar_spinlock)
            verificar(pt_spinlock_lock(&spinlock));
        else
            verificar(pt_mutex_lock(&mutex));

        // Inicio da regiao critica
        contador++;
        simular_trabalho(tarefa->duracao_us);
        // Fim da regiao critica

        if (tarefa->usar_spinlock)
            verificar(pt_spinlock_unlock(&spinlock));
        else
            verificar(pt_mutex_unlock(&mutex));
    }

    return NULL;
}

// Executa uma rodada experimental
static double executar_rodada(
    int n_threads,
    int duracao_us,
    int usar_spinlock
) {
    pt_thread_t threads[MAX_THREADS];
    tarefa_t tarefas[MAX_THREADS];

    contador = 0;

    if (usar_spinlock)
        verificar(pt_spinlock_init(&spinlock));
    else
        verificar(pt_mutex_init(&mutex));

    for (int i = 0; i < n_threads; i++) {
        tarefas[i].aquisicoes =
            TOTAL_AQUISICOES / n_threads;

        tarefas[i].duracao_us = duracao_us;
        tarefas[i].usar_spinlock = usar_spinlock;
    }

    uint64_t inicio = agora_ns();

    for (int i = 0; i < n_threads; i++) {
        verificar(pt_thread_create(&threads[i], executar, &tarefas[i]));
    }

    for (int i = 0; i < n_threads; i++) {
        verificar(pt_thread_join(threads[i], NULL));
    }

    uint64_t fim = agora_ns();

    if (usar_spinlock)
        verificar(pt_spinlock_destroy(&spinlock));
    else
        verificar(pt_mutex_destroy(&mutex));

    if (contador != TOTAL_AQUISICOES) {
        fprintf(stderr,
            "ERRO: contador %ld, esperado %d\n",
            contador, TOTAL_AQUISICOES);

        exit(EXIT_FAILURE);
    }

    return (double)(fim - inicio) / 1e9;
}

int main(void) {

    FILE *csv = fopen(
        "dados/linux/benchmark_linux.csv", "wx");

    if (!csv) {
        perror("Erro ao criar CSV");
        return 1;
    }

    fprintf(csv,
        "cenario,mecanismo,threads,repeticao,"
        "aquisicoes,tempo_s,vazao\n");

    const char *cenarios[] = {
        "curta", "media", "longa"
    };

    const int duracoes[] = {
        0, 1, 100
    };

    const int quantidades[] = {
        1, 2, 4, 8, 16
    };

    printf("===== BENCHMARK LINUX =====\n");
    printf("Aquisicoes por rodada: %d\n",
           TOTAL_AQUISICOES);
    printf("Repeticoes: %d\n\n", REPETICOES);

    int progresso = 0;

    for (int c = 0; c < 3; c++) {
        for (int t = 0; t < 5; t++) {
            for (int m = 0; m < 2; m++) {

                const char *mecanismo =
                    m ? "spinlock" : "mutex";

                for (int r = 1; r <= REPETICOES; r++) {

                    double tempo = executar_rodada(
                        quantidades[t], duracoes[c], m
                    );

                    double vazao =
                        TOTAL_AQUISICOES / tempo;

                    fprintf(csv,
                        "%s,%s,%d,%d,%d,%.9f,%.2f\n",
                        cenarios[c],
                        mecanismo,
                        quantidades[t],
                        r,
                        TOTAL_AQUISICOES,
                        tempo,
                        vazao);

                    fflush(csv);

                    progresso++;

                    printf(
                        "[%d/900] %s | %s | "
                        "%d threads | repeticao %d | "
                        "%.6f s\n",
                        progresso,
                        cenarios[c],
                        mecanismo,
                        quantidades[t],
                        r,
                        tempo
                    );

                    fflush(stdout);
                }
            }
        }
    }

    fclose(csv);

    printf("\nBenchmark concluido!\n");
    printf("Resultados: dados/linux/benchmark_linux.csv\n");

    return 0;
}

