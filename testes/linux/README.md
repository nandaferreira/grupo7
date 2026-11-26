# Mutex e Spinlock no Linux

## 1. Objetivo

Demonstrar o funcionamento dos mecanismos de sincronização
Mutex e Spinlock no Linux, utilizando a biblioteca POSIX
Threads (pthread), com análise das chamadas de sistema Futex.

## 2. Ambiente experimental

| Característica | Configuração |
|---|---|
| Sistema operacional | Ubuntu 26.04.1 LTS |
| Kernel | 7.0.0-44-generic |
| Processador | Intel Celeron 5205U |
| Frequência máxima | 1,90 GHz |
| Núcleos físicos | 2 |
| Processadores lógicos | 2 |
| Arquitetura | x86_64 |
| Memória RAM | 7,1 GiB |
| Compilador | GCC 15.2.0 |
| Strace | 6.19 |

## 3. Implementação

O programa cria quatro threads, responsáveis por realizar
100.000 incrementos cada uma em uma variável compartilhada.

A região crítica é protegida utilizando Mutex ou Spinlock.

### Mutex

Utiliza operações atômicas e pode recorrer ao mecanismo
Futex para bloquear e despertar threads em situações de
contenção.

### Spinlock

Utiliza espera ocupada (busy waiting), realizando novas
tentativas de aquisição do lock enquanto recebe tempo de CPU.

## 4. Compilação

A partir da raiz do repositório:

```bash
cd testes/linux
gcc -std=c11 -O2 -Wall -Wextra -pthread \
    demo_mutex_spinlock_linux.c -o demo
```

## 5. Execução

### Mutex

```bash
./demo mutex
```

### Spinlock

```bash
./demo spin
```

### Comparação dos tempos

```bash
time ./demo mutex
time ./demo spin
```

### Rastreamento das chamadas Futex

```bash
strace -f -c -e trace=futex ./demo mutex
strace -f -c -e trace=futex ./demo spin
```

## 6. Resultados preliminares

Testes realizados em 25/11/2026.

| Métrica | Mutex | Spinlock |
|---|---:|---:|
| Threads | 4 | 4 |
| Incrementos por thread | 100.000 | 100.000 |
| Contador esperado | 400.000 | 400.000 |
| Contador final | 400.000 | 400.000 |
| Tempo total | 40 ms | 29 ms |
| Tempo em modo usuário | 30 ms | 33 ms |
| Tempo em modo kernel | 18 ms | 4 ms |
| Chamadas Futex | 341 | 2 |

## 7. Evidências experimentais

Os logs das chamadas de sistema estão disponíveis em:

- [Mutex](../../dados/linux/mutex_futex.log)
- [Spinlock](../../dados/linux/spin_futex.log)

No Mutex foram observadas operações FUTEX_WAIT_PRIVATE,
FUTEX_WAKE_PRIVATE e retornos EAGAIN.

No Spinlock, as chamadas Futex observadas são compatíveis
com a espera pela finalização das threads (pthread_join).

## 8. Demonstração automatizada

```bash
bash apresentar_linux.sh
```

## 9. Considerações

Ambos os mecanismos produziram o contador esperado.

O Mutex pode bloquear threads em situações de contenção,
enquanto o Spinlock utiliza espera ocupada.

Os resultados são preliminares e correspondem a execuções
individuais. Os tempos e as chamadas Futex foram medidos
em execuções separadas.

Os logs detalhados também correspondem a execuções
distintas dos resumos estatísticos.

Esta demonstração não substitui o benchmark oficial
multiplataforma do trabalho.

## 10. Referências

- Aula 04 — Threads.
- Aula 05 — Concorrência, Partes 1 e 2.
- Aula 03 — Escalonamento de Processos.
- Silberschatz, Galvin e Gagne — Fundamentos de Sistemas Operacionais.
