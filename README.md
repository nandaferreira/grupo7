# Grupo 7 — Threads, Mutex e Spinlock

**GBC045 — Sistemas Operacionais | FACOM/UFU | 2026/2**

## Objetivo

Implementar e avaliar mecanismos nativos de sincronização entre threads no Linux e no Windows. O projeto compara Mutex e Spinlock na proteção de um contador compartilhado, verificando a exclusão mútua e investigando os efeitos da contenção, do número de threads e da duração da seção crítica sobre o desempenho.

## Integrantes

- Alicia da Silva Feliciano
- Fernanda Ferreira de Melo
- Geovanna Miguel Lamouni

## Implementações

| Recurso | Linux (POSIX) | Windows (Win32) |
|---|---|---|
| Threads | `pthread_create`, `pthread_join` | `CreateThread`, `WaitForSingleObject` |
| Mutex | `pthread_mutex_t` | `CRITICAL_SECTION` |
| Spinlock | `pthread_spinlock_t` | `InterlockedExchange`, `YieldProcessor` |
| Temporização | `clock_gettime(CLOCK_MONOTONIC)` | `QueryPerformanceCounter` |

O Mutex pode bloquear a thread sob contenção; o Spinlock utiliza espera ocupada. No Linux, o Mutex pode recorrer a Futex. A interface `pt_*`, em `code/platform_thread.h` e `code/platform_thread.c`, recebeu suporte POSIX, validado funcionalmente no Linux em branch de integração. Os resultados Linux originais foram coletados diretamente com pthreads; a disponibilidade da integração na branch principal deve ser conferida.

## Estrutura do repositório

```text
grupo7/
├── code/
│   ├── platform_thread.h
│   └── platform_thread.c
├── testes/
│   └── linux/
│       ├── README.md
│       ├── demo_mutex_spinlock_linux.c
│       └── apresentar_linux.sh
├── experimentos/
│   ├── mutex.c
│   ├── spinlock.c
│   ├── benchmark_linux.c
│   ├── gerar_graficos_linux.py
│   └── graficos/linux/
├── dados/
│   ├── dados_brutos_mutex.csv
│   ├── dados_brutos_spinlock.csv
│   ├── estatisticas_mutex.csv
│   ├── estatisticas_spinlock.csv
│   ├── graficos_vazao.svg
│   └── linux/
│       ├── benchmark_linux.csv
│       ├── estatisticas_linux.csv
│       ├── mutex_futex.log
│       └── spin_futex.log
└── video/
```

A estrutura acima reúne os arquivos produzidos pelo grupo; os resultados Linux foram enviados à branch `feature/linux-benchmark`. Confira se as branches foram integradas antes de reproduzir os comandos na `main`.

## Metodologia experimental

| Parâmetro | Configuração |
|---|---|
| Número de threads | 1, 2, 4, 8 e 16 |
| Trabalho simulado na seção crítica | 0, 1 e 100 µs |
| Repetições | 30 por configuração |
| Aquisições por rodada no Linux e Mutex Windows | 16.000 (fixas) |
| Aquisições no Spinlock Windows original | 10.000 por thread |
| Métricas | Tempo total, vazão média e desvio padrão amostral |

A vazão de cada rodada corresponde ao total de aquisições dividido pelo tempo em segundos. A média e o desvio padrão são calculados sobre as vazões individuais. A coleta Linux compreende 900 medições; cada benchmark Windows compreende 450 medições. Nos CSVs originais do Windows, os rótulos `media_1ms` e `longa_100ms` correspondem, de fato, a 1 e 100 microssegundos.

## Reprodução no Linux

Utilize Ubuntu nativo com GCC, Python e as bibliotecas de análise:

```bash
sudo apt install build-essential strace python3-pandas python3-matplotlib
```

Consulte o [README dos testes Linux](testes/linux/README.md) para a demonstração, rastreamento Futex e teste funcional da biblioteca.

Na raiz do repositório, para compilar a versão original do benchmark (POSIX direto):

```bash
gcc -std=c11 -O2 -Wall -Wextra -pthread \
    experimentos/benchmark_linux.c -o /tmp/benchmark_linux
```

Se estiver usando a versão integrada à biblioteca `pt_*`, inclua `code/platform_thread.c` na compilação:

```bash
gcc -std=c11 -O2 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -pthread \
    experimentos/benchmark_linux.c code/platform_thread.c -o /tmp/benchmark_linux
```

Para repetir a coleta sem sobrescrever os resultados existentes, execute em uma pasta temporária:

```bash
VALIDACAO=$(mktemp -d)
mkdir -p "$VALIDACAO/dados/linux"
(cd "$VALIDACAO" && /tmp/benchmark_linux)
wc -l "$VALIDACAO/dados/linux/benchmark_linux.csv"
```

São esperadas 901 linhas, incluindo o cabeçalho. Para gerar os gráficos a partir do CSV já versionado, execute na raiz do repositório:

```bash
python3 experimentos/gerar_graficos_linux.py
xdg-open experimentos/graficos/linux/
```

## Reprodução no Windows

Utilize Windows nativo com GCC/MinGW disponível no PATH. Os programas utilizam a API Win32; não é necessário usar WSL ou Cygwin. Na raiz do repositório, em PowerShell:

```powershell
gcc -O2 -Wall -Wextra experimentos/mutex.c code/platform_thread.c -o experimentos/mutex.exe -lm
gcc -O2 -Wall -Wextra experimentos/spinlock.c code/platform_thread.c -o experimentos/spinlock.exe -lm
```

Após confirmar a compilação, execute em uma pasta separada para preservar os CSVs originais:

```powershell
$raiz = (Get-Location).Path
$destino = Join-Path ([System.IO.Path]::GetTempPath()) ([guid]::NewGuid().ToString())
New-Item -ItemType Directory -Path (Join-Path $destino 'dados') -Force | Out-Null
Push-Location $destino
try {
    & (Join-Path $raiz 'experimentos/mutex.exe')
    & (Join-Path $raiz 'experimentos/spinlock.exe')
} finally {
    Pop-Location
}
Get-ChildItem (Join-Path $destino 'dados')
```

O ambiente de hardware e as versões das ferramentas Windows não foram informados nos materiais disponíveis.

## Resultados

Os gráficos Linux apresentam a vazão média em aquisições por segundo, com barras de erro de um desvio padrão amostral.

| Cenário | Linux | Windows |
|---|---|---|
| Curto (≈ 0 µs de trabalho adicional) | Spinlock com maior vazão média na maioria dos pontos | Spinlock predominante |
| Médio (1 µs) | Mutex com maior vazão média a partir de 2 threads | Spinlock predominante nos resultados originais |
| Longo (100 µs) | Mutex com vantagem crescente sob concorrência | Inversão observada na região longa, entre 8 e 16 threads |

### Gráficos Linux

| Curta | Média | Longa |
|---|---|---|
| [Gráfico](experimentos/graficos/linux/curta.png) | [Gráfico](experimentos/graficos/linux/media.png) | [Gráfico](experimentos/graficos/linux/longa.png) |

- [Dados brutos Linux](dados/linux/benchmark_linux.csv)
- [Estatísticas Linux](dados/linux/estatisticas_linux.csv)
- [Gráficos Windows](dados/graficos_vazao.svg)

As diferenças de vazão são descritivas; as barras de desvio padrão não constituem um teste de significância. A carga variável do Spinlock Windows, as diferenças de hardware e as implementações impedem atribuir a inversão exclusivamente ao sistema operacional. Os dados originais foram preservados.

## Relatório e vídeo

O relatório técnico consolidado contém os seis gráficos e a discussão comparativa. O diretório [`video/`](video/) é destinado ao material audiovisual. Consulte os arquivos da entrega do grupo para a versão final do relatório e do vídeo.

## Referências

- NASCIMENTO, Marcelo Zanchetta do. Aulas 03 (Escalonamento), 04 (Threads) e 05 (Concorrência). GBC045, FACOM/UFU, 2026.
- SILBERSCHATZ, Abraham; GALVIN, Peter Baer; GAGNE, Greg. *Fundamentos de Sistemas Operacionais*.
- [POSIX.1-2017 — The Open Group](https://pubs.opengroup.org/onlinepubs/9699919799/).
- [Microsoft Learn — Critical Section Objects](https://learn.microsoft.com/en-us/windows/win32/sync/critical-section-objects).

- NASCIMENTO, Marcelo Zanchetta do. Aulas 03 (Escalonamento), 04 (Threads) e 05 (Concorrência). GBC045, FACOM/UFU, 2026.
- SILBERSCHATZ, Abraham; GALVIN, Peter Baer; GAGNE, Greg. Fundamentos de Sistemas Operacionais.
- POSIX.1-2017 — The Open Group.
- Microsoft Learn — Critical Section Objects.
