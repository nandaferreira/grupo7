#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../code/platform_thread.h"
#define TOTAL_AQUISICOES 16000
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
static pt_mutex_t mutex_experimento;
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
        pt_mutex_lock(&mutex_experimento);
        contador_compartilhado++; //seção crítica
        simular_trabalho_ms(dados->duracao_ms);
        pt_mutex_unlock(&mutex_experimento);// fim secao 
    }
    return NULL;
}

//executar uma rodada completa para N threads

static double executar_rodada(int num_threads, double duracao_ms){
    pt_thread_t threads[MAX_THREADS];
    tarefas_t args[MAX_THREADS];
    LARGE_INTEGER t_inicio, t_fim;

    contador_compartilhado = 0;
    pt_mutex_init(&mutex_experimento);

    int aquisicoes_por_thread = TOTAL_AQUISICOES / num_threads;
    for(int i= 0; i<num_threads; i++){
        args[i].aquisicoes = aquisicoes_por_thread;
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
    pt_mutex_destroy(&mutex_experimento);

    if(contador_compartilhado != TOTAL_AQUISICOES){
        fprintf(stderr, "erro - esperado: %d | obtido %ld\n", TOTAL_AQUISICOES, contador_compartilhado);
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
    FILE *csv_bruto = fopen("dados/dados_brutos_mutex.csv", "w");
    FILE *csv_estatisticas = fopen("dados/estatisticas_mutex.csv", "w");

    if(csv_bruto == NULL || csv_estatisticas == NULL){
        fprintf(stderr, "erro ao abrir arquivos em 'dados/'\n");
        return 1;
    }

    fprintf(csv_bruto, "cenario,num_threads,repeticao, total_aquisicoes,tempo_total_segundos,vazao\n");
    fprintf(csv_estatisticas, "cenario,num_threads,repeticoes,media_tempo_segundos,desvio_tempo_egundos,media_vazao,desvio_vazao\n");

    const char *nomes_cenarios[3] = {"curta", "media_1ms", "longa_100ms"};
    const double duracoes_ms[3] = {0.0, 1.0, 100.0};
    const int lista_threads[5] = {1,2,4,8,16};

    printf("--- desemepenho mutex (Win32) --\n");
    printf("total de aquisicoes por rodada: %d | repeticoes: %d\n\n", TOTAL_AQUISICOES, REPETICOES);

    for(int c= 0; c<3; c++){
        for(int t=0; t<5; t++){
            int n_threads = lista_threads[t];
            double tempos[REPETICOES];
            double vazoes[REPETICOES];

            for(int r=0; r < REPETICOES; r++){
                double tempo_segundos = executar_rodada(n_threads, duracoes_ms[c]);
                double vazao = (double)TOTAL_AQUISICOES / tempo_segundos;
                tempos[r] = tempo_segundos;
                vazoes[r] = vazao;

                fprintf(csv_bruto, "%s,%d,%d,%d,%.9f,%.2f\n", nomes_cenarios[c], n_threads, r+1, TOTAL_AQUISICOES, tempo_segundos, vazao);
            }
            fflush(csv_bruto);
            
            double media_tempo = calcular_media(tempos, REPETICOES);
            double tempo_desvioP = calcula_desvioP(tempos, REPETICOES, media_tempo);
            double media_vazao = calcula_media(vazoes, REPETICOES);
            double vazao_desvioP = calcula_desvioP(vazoes, REPETICOES,media_vazao);

            fprintf(csv_estatisticas, "%s,%d,%d,%.9f,%.9f,%.2f,%.2f\n", nomes_cenarios[c], n_threads, REPETICOES, media_tempo, tempo_desvioP, media_vazao, vazao_desvioP);
            fflush(csv_estatisticas);
            //adicionar prints
        }
    }


}