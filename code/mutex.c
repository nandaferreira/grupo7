#include <windows.h>
#include <stdio.h>
#define THREADS_TRABALHO 4
HANDLE m_hMutex = NULL;


int main(){

    HANDLE aThread[THREADS_TRABALHO];
    DWORD idThread;
    
    //criacao da thread com o id do SO
    m_hMutex = CreateMutex(
        NULL,
        TRUE,
        "idMutexdoSO"
    );

    if(m_hMutex == NULL){
        printf("erro de criacao mutex: %d\n", GetLastError());
        return 1;
    }

    //threads de trabalho

    for(int i = 0; i < THREADS_TRABALHO; i++){
        aThread[i] = CreateThread(
            NULL,
            0, //tamanho padrao stack
            (LPTHREAD_START_ROUTINE)WriteToDataBase, // tem que criar uma funcao que é executada pela thread?
            NULL,
            0,
            &idThread);

        if(aThread[i] == NULL){
            printf("erro de criacao da thread: %d\n", GetLastError());
            return 1;
        }
    }
    //espera todas as threads terminar

    WaitForMultipleObjects(THREADS_TRABALHO, aThread, TRUE, INFINITE);

    //fecha thread e handler de mutex

    for(int i= 0; i < THREADS_TRABALHO; i++){
        CloseHandle(aThread[i]);
    }
    CloseHandle(m_hMutex);
    
    return 0;
}

DWORD WINAPI WriteToDataBase(LPVOID lpParam){
    //lpParam nao usado

    UNREFRENCED_PARAMETER(lpParam);
    DWORD dwCount =0, dwWaitResult;

    //solcita propriedade do mutex

    while(dwCount < 20){
        dwWaitResult = WaitForSingleObject(
            m_hMutex,
            INFINITE
        );
        switch(dwWaitResult){
            case WAIT_OBJECT_0:
                __try {
                    printf("thread %d escrevendo na base de dados...\n", GetCurrentThreadId());
                    dwCount++;
                }
                __finally {
                    //libera propriedade sobre o mutex
                    if(!ReleaseMutex(m_hMutex)){
                        printf("erro de liberacao do mutex: %d\n", GetLastError());
                    }
                }
                break;
            
            case WAIT_ABANDONED:
                printf("mutex abandonado: %d\n", GetLastError());
                return FALSE;
        }
    }
    return TRUE;
}




//-------------------------------------------------------------------------------------------------------------------------------------------

/* 
1- Declarar variável de nível de módulo para persistência do handle static HANDLE m_hMutex = NULL;  
2 - Criar mutex com identificador único no nível do SO m_hMutex = CreateMutex(     NULL,              // Atributos de segurança padrão     TRUE,              // Proprietário inicial: TRUE adquire o mutex     "MyUniqueMutexIdInOS"  // Identificador de string único );  
3 - Verificar se o mutex já existia (outra instância o detém) if (GetLastError() == ERROR_ALREADY_EXISTS) { //Mutex já criado por outro processo     
4- Tratar adequadamente com base na lógica da aplicação } */

/* criar, bloquear e destruir 
Qualquer fio que bloqueia com sucesso o mutex torna-se o dono do mutex até que ele desbloqueie o mutex. Qualquer fio que tente bloquear o mutex depois tem que esperar até que o dono desbloqueie o mutex. Quando o proprietário desbloqueia o mutex, o controle é retornado a um fio de espera com esse fio tornando-se o dono do mutex. Pode haver apenas um dono de um mutex de cada vez.*/

/*Projeto: implementaçao de mecanismos de sincronização de threads utilizndo a API nativa WIN32 API. Mecanismo a implementar: mutex
programação concorrente em nível de aplicação usando APIs nativas de threading;
mecanismos de sincronização (mutex, variável de condição, semáforo, spinlock);
comparação empírica de escalonadores e subsistemas de threads entre SOs;
metodologia experimental (repetições, média, desvio padrão, isolamento de ruído)

Projeto consiste em implementar uma camada de abstração de threads (plataform_thread.h/.c) que expoe uma API única para criaçao de threads e sincronizaçào, traduzindo internamente para a API nativa de cada SO. API Win32 nativa (CreateThread, CRITICAL_SECTION, CONDITION_VARIABLE) em Windows. 

Não é permitido usar winpthreads como atalho, pois o objetivo é confrontar duas famílias de API de sincronizaçào distintas, e não a mesma API rodando sobre kernels diferentes.

implementação:
- Camada de abstração de threads funcional em Windows
- Mecanismo de sincronizaçao: mutex
- Programa de usuário para testar cada operaçao

Benchmark: no min 30 repeticoes por configuracao, com dados brutos preservados

Implementacao: será explorado o pt_mutex_t (sobre pthread_mutex_t ou CRITICAL_SECTION)

Experimento: N threads executam, juntas, um total fixo de aquisiçoes do lock. Em cada aquisicao, a thread entra na secao critica, executa um trabalho e sai

Numero de threads: 1, 2, 4, 8, 16
Duracao da seçao critica: curta (apenas incrementar o contador), media (cerca de 1 μs de trabalho) e longa (cerca de 100 μs)

Primitiva: mutex

Metricas: tempo total e vazao (aquisicoes por segundo), com media e desvio padrao das 30 repeticoes.


*/