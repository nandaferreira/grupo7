#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../code/platform_thread.h"
#define AQUISICOES_POR_THREAD 10000
#define REPETICOES 30
#define MAX_THREADS 16

typedef enum {
    CENARIO_CURTA = 0,
    CERNARIO_MEDIA = 1,
    CERNARIO_LONGA = 2
} cenario_t;

typedef struct{
    int aquisicoes;
    double duracao_ms;
}tarefas_t;

//recursos compartilhados
static long contador_compartilhado = 0;
static pt_spinlock_t spinlock_experimento;
static LARGE_INTEGER freq_cpu;

//simular trabalhho na secao critica
static void simular_trabalho_ms(double microsseg){
    if(microsseg <= 0.0){return;}
    LARGE_INTEGER inicio, atual;
    QueryPerformanceCounter(&inicio);

    double decorrido_ms = 0.0;
    while(decorrido_ms < microsseg){
        QueryPerformanceCounter(&atual);
        decorrido_ms = ((double)(atual.QuadPart - inicio.QuadPart) *1e6)/ (double)freq_cpu.QuadPart;
    }
}

void *task_experimento(void *arg){
    tarefas_t *dados = (tarefas_t*)arg;

    for(int i = 0; i < dados->aquisicoes; i++){
        pt_spinlock_lock(&spinlock_experimento);
        contador_compartilhado++; //seção crítica
        simular_trabalho_ms(dados->duracao_ms);
        pt_spinlock_unlock(&spinlock_experimento);// fim secao 
    }
    return NULL;
}

//executar uma rodada completa para N threads

static double executar_rodada(int num_threads, double duracao_ms){
    pt_thread_t threads[MAX_THREADS];
    tarefas_t args[MAX_THREADS];
    LARGE_INTEGER t_inicio, t_fim;

    contador_compartilhado = 0;
    pt_spinlock_init(&spinlock_experimento);

    long total_aquisicoes = (long)num_threads * AQUISICOES_POR_THREAD;
    for(int i= 0; i<num_threads; i++){
        args[i].aquisicoes = AQUISICOES_POR_THREAD;
        args[i].duracao_ms = duracao_ms;
    }
    QueryPerformanceCounter(&t_inicio);
    for(int i = 0; i<num_threads; i++){
        if(pt_thread_create(&threads[i], task_experimento, &args[i]) !=0 ){
            fprintf(stderr, "erro de criacao da thread %d\n", i);
            exit(1);
        }
    }
    for(int i= 0; i < num_threads; i++){
        if(pt_thread_join(threads[i], NULL) != 0){
            fprintf(stderr, "erro de aguarde da thread %d\n", i);
            exit(1);
        }
    }
    QueryPerformanceCounter(&t_fim);
    pt_spinlock_destroy(&spinlock_experimento);

    if(contador_compartilhado != total_aquisicoes){
        fprintf(stderr, "erro - esperado: %ld | obtido %ld\n", total_aquisicoes, contador_compartilhado);
        exit(1);
    }
    return(double)(t_fim.QuadPart - t_inicio.QuadPart)/ (double)freq_cpu.QuadPart;
}

static double calcula_media(const double *valores, int n){
    double soma = 0.0;
    for(int i = 0; i <n; i++){
        soma += valores[i];
    }
    return soma/n;
}

static double calcula_desvioP(const double *valores, int n, double media){
    if(n <= 1){return 0.0;}
    double soma = 0.0;
    for(int i=0; i<n ; i++){
        double diff = valores[i] - media;
        soma += diff*diff;
    }
    return sqrt(soma /(n-1));
}

int main(){
    QueryPerformanceFrequency(&freq_cpu);
    FILE *csv_bruto = fopen("dados/dados_brutos_spinlock.csv", "w");
    FILE *csv_estatisticas = fopen("dados/estatisticas_spinlock.csv", "w");

    if(csv_bruto == NULL || csv_estatisticas == NULL){
        fprintf(stderr, "erro ao abrir arquivos em 'dados/'\n");
        return 1;
    }

    fprintf(csv_bruto, "cenario,num_threads,repeticao, total_aquisicoes,tempo_total_segundos,vazao\n");
    fprintf(csv_estatisticas, "cenario,num_threads,repeticoes,media_tempo_segundos,desvio_tempo_egundos,media_vazao,desvio_vazao\n");

    const char *nomes_cenarios[3] = {"curta", "media_1ms", "longa_100ms"};
    const double duracoes_ms[3] = {0.0, 1.0, 100.0};
    const int lista_threads[5] = {1,2,4,8,16};

    printf("--- desemepenho spinlock (Win32) --\n");
    printf("aquisicoes por thread: %d | repeticoes: %d\n\n", AQUISICOES_POR_THREAD, REPETICOES);

    for(int c= 0; c<3; c++){
        for(int t=0; t<5; t++){
            int n_threads = lista_threads[t];
            long total_aquisicoes = (long)n_threads * AQUISICOES_POR_THREAD;
            double tempos[REPETICOES];
            double vazoes[REPETICOES];

            for(int r=0; r < REPETICOES; r++){
                double tempo_segundos = executar_rodada(n_threads, duracoes_ms[c]);
                double vazao = (double)total_aquisicoes / tempo_segundos;
                tempos[r] = tempo_segundos;
                vazoes[r] = vazao;

                fprintf(csv_bruto, "%s,%d,%d,%ld,%.9f,%.2f\n", nomes_cenarios[c], n_threads, r+1, total_aquisicoes, tempo_segundos, vazao);
            }
            fflush(csv_bruto);
            
            double media_tempo = calcula_media(tempos, REPETICOES);
            double tempo_desvioP = calcula_desvioP(tempos, REPETICOES, media_tempo);
            double media_vazao = calcula_media(vazoes, REPETICOES);
            double vazao_desvioP = calcula_desvioP(vazoes, REPETICOES,media_vazao);

            fprintf(csv_estatisticas, "%s,%d,%d,%.9f,%.9f,%.2f,%.2f\n", nomes_cenarios[c], n_threads, REPETICOES, media_tempo, tempo_desvioP, media_vazao, vazao_desvioP);
            fflush(csv_estatisticas);

            printf("[%s | %2d threads] tempo: %.6f s (+/- %.6f) | vazao: %.2f operacoes/segundo (+/- %.2f)\n", nomes_cenarios[c], n_threads, media_tempo, tempo_desvioP, media_vazao, vazao_desvioP);
        }
        printf("---------------------\n");
    }

    fclose(csv_bruto);
    fclose(csv_estatisticas);
    printf("\n dados salvos com sucesso. verificar dados/");
    return 0;
}