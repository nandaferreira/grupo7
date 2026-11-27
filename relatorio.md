# Relatório — Sincronização de threads com Mutex e Spinlock

## 1. Introdução e motivação do projeto

A motivação desse projeto é demonstrar e comparar mecanismos de sincronização entre threads em Linux e Windows, usando as APIs nativas disponíveis em cada sistema. A intenção é proteger uma região crítica que atualiza um contador compartilhado, observar o comportamento dos mecanismos sob concorrência e discutir como o número de threads e o custo da região crítica afetam o tempo de execução.

Sem sincronização, duas ou mais threads podem ler e alterar o contador ao mesmo tempo, causando condições de corrida e perda de incrementos. O mutex e o spinlock asseguram acesso exclusivo à região crítica, mas tratam a espera de maneiras diferentes:

- **Mutex:** uma thread que não consegue adquirir o bloqueio pode ser suspensa até que ele seja liberado. Isso evita consumir continuamente CPU durante esperas mais longas, embora o bloqueio e o despertar tenham custos.
- **Spinlock:** a thread repete tentativas de aquisição enquanto o bloqueio está ocupado. Essa espera ativa pode ser vantajosa quando a espera é muito curta, mas consome CPU e pode aumentar a contenção.

Para a implementação em Windows, a biblioteca de abstração de mutex utiliza a _CRITICAL_SECTION_, com: 
- _InitializeCriticalSection_ 
- _EnterCriticalSection_
- _LeaveCriticalSection_

 Já o spinlock usa as operações atômicas da Win32 _InterlockedExchange_ com: 
 - _YieldProcessor_
 
 A demonstração Linux utiliza _pthread_mutex_ e _pthread_spinlock_; a implementação de mutex POSIX pode recorrer a Futex quando há contenção.

Os experimentos verificam a correção no final, onde o contador  corresponde ao total de incrementos previsto. Os tempos foram interpretados junto com o tamanho da carga, a configuração da máquina e o método usado para medir.
***
## 2. Linux — configuração e implementação

### 2.1 Configuração do ambiente

Configuração registrada para a máquina Linux:

| Característica | Configuração |
|---|---|
| Sistema operacional | Ubuntu 26.04.1 LTS |
| Kernel | 7.0.0-44-generic |
| Processador | Intel Celeron 5205U |
| Frequência máxima | 1,90 GHz |
| Núcleos físicos / processadores lógicos | 2 / 2 |
| Arquitetura | x86_64 |
| Memória RAM | 7,1 GiB |
| Compilador | GCC 15.2.0 |
| Ferramenta de rastreamento | strace 6.19 |

### 2.2 Implementação e execução

A demonstração em  cria quatro threads onde cada uma realiza 100.000 incrementos no contador compartilhado, o que resulta em um total de  400.000 incrementos. O programa seleciona o mecanismo pela linha de comando (`mutex` ou `spin`), valida as chamadas POSIX e compara o contador final com o valor esperado.

O mutex é inicializado estaticamente com _PTHREAD_MUTEX_INITIALIZER_. O spinlock é inicializado com _pthread_spin_init_ no modo _PTHREAD_PROCESS_PRIVATE_. Em ambos os casos, a aquisição e a liberação delimitam apenas o incremento do contador. A execução pode ser feita com `./demo mutex` ou `./demo spin`; o README em `testes/linux/README.md` registra os comandos de compilação e de rastreamento com `strace`.

### 2.3 Resultados observados

Na execução preliminar registrada em 25/11/2026, ambos os mecanismos chegaram ao contador esperado:

| Métrica | Mutex | Spinlock |
|---|---:|---:|
| Threads | 4 | 4 |
| Incrementos por thread | 100.000 | 100.000 |
| Contador final | 400.000 | 400.000 |
| Tempo total registrado | 40 ms | 29 ms |
| Tempo de CPU em modo usuário | 30 ms | 33 ms |
| Tempo de CPU em modo kernel | 18 ms | 4 ms |
| Chamadas Futex registradas | 341 | 2 |

O menor tempo total observado para o spinlock é compatível com uma região crítica muito curta: suspender e despertar threads pode custar mais do que repetir brevemente a tentativa de aquisição. O maior número de chamadas Futex no caso do mutex é compatível com esperas e despertares sob contenção. As chamadas Futex observadas no spinlock são compatíveis com a espera de finalização das threads (`pthread_join`), não necessariamente com a aquisição do spinlock.

Esses números são resultados preliminares, não uma comparação conclusiva: correspondem a execuções individuais, e os tempos e rastreamentos foram coletados em execuções separadas. Os tempos de CPU são contabilizados entre as threads e podem somar mais que o tempo de parede; portanto, não devem ser somados para reproduzir o tempo total. Os logs estão em `dados/linux/mutex_futex.log` e `dados/linux/spin_futex.log`.

### 2.4 Espaço para configuração adicional / evidências

- **Comando e opções de compilação utilizados:** [Preencher, se diferentes dos registrados no README.]
- **Data e horário da coleta:** [Preencher.]
- **Condições da máquina durante a coleta (carga, energia, processos concorrentes):** [Preencher.]
- **Capturas de tela, saídas ou evidências adicionais:** [Inserir aqui.]

## 3. Windows — configuração das máquinas e resultados

Os arquivos de benchmark Windows (`experimentos/mutex.c` e `experimentos/spinlock.c`) registram 30 repetições para 1, 2, 4, 8 e 16 threads, em três cenários, usando `QueryPerformanceCounter`. As tabelas abaixo resumem os CSVs estatísticos presentes em `dados/`. As informações de hardware e sistema Windows não estão registradas nos arquivos disponíveis e devem ser preenchidas pelos responsáveis.

### 3.1 Máquina Windows responsável pelo Mutex

#### Configuração utilizada

| Característica | Configuração |
|---|---|
| Sistema operacional e versão | [Preencher] |
| Processador e modelo | [Preencher] |
| Núcleos físicos / processadores lógicos | [Preencher] |
| Memória RAM | [Preencher] |
| Compilador e versão / opções de compilação | [Preencher] |
| Data e condições da coleta | [Preencher] |

#### Implementação e resultados observados

O benchmark protege o incremento do contador com a `CRITICAL_SECTION` da camada de abstração. O total de aquisições por rodada é fixo em 16.000, distribuídas entre as threads. Cada cenário registra a média e o desvio-padrão de 30 execuções.

| Cenário registrado no CSV | Threads | Média do tempo total (s) | Desvio-padrão (s) |
|---|---:|---:|---:|
| curta | 1 | 0,000727 | 0,000221 |
| curta | 2 | 0,001786 | 0,000275 |
| curta | 4 | 0,003062 | 0,000436 |
| curta | 8 | 0,003825 | 0,000312 |
| curta | 16 | 0,003981 | 0,000444 |
| media_1ms | 1 | 0,017347 | 0,000511 |
| media_1ms | 2 | 0,021968 | 0,006267 |
| media_1ms | 4 | 0,021010 | 0,000404 |
| media_1ms | 8 | 0,021509 | 0,000320 |
| media_1ms | 16 | 0,021841 | 0,000265 |
| longa_100ms | 1 | 1,606526 | 0,009487 |
| longa_100ms | 2 | 1,622304 | 0,004622 |
| longa_100ms | 4 | 1,626632 | 0,002110 |
| longa_100ms | 8 | 1,630360 | 0,002004 |
| longa_100ms | 16 | 1,632036 | 0,006078 |

Na amostra disponível, o tempo do cenário curto cresce conforme aumentam as threads, apesar de o número total de aquisições permanecer fixo. Nos cenários de maior duração, a região crítica serializada domina o tempo: aumentar o número de threads não reduz o trabalho que precisa ser executado sob exclusão mútua. As diferenças pequenas entre alguns números de threads devem ser avaliadas considerando o desvio-padrão e o ambiente de execução.

**Atenção à unidade dos cenários:** o código converte o contador de alta resolução para microssegundos (`* 1e6`), mas compara esse valor com parâmetros chamados `duracao_ms` e nomeia os cenários como `media_1ms` e `longa_100ms`. Assim, os valores passados `1` e `100` são efetivamente comparados como microssegundos, não como milissegundos. Os rótulos existentes não representam as durações nominais sugeridas pelos nomes; isso deve ser levado em conta ao interpretar os resultados, sem alterar a implementação neste relatório.

### 3.2 Máquina Windows responsável pelo Spinlock

#### Configuração utilizada

| Característica | Configuração |
|---|---|
| Sistema operacional e versão | [Preencher] |
| Processador e modelo | [Preencher] |
| Núcleos físicos / processadores lógicos | [Preencher] |
| Memória RAM | [Preencher] |
| Compilador e versão / opções de compilação | [Preencher] |
| Data e condições da coleta | [Preencher] |

#### Implementação e resultados observados

O benchmark utiliza o spinlock da camada de abstração: uma operação atômica Win32 tenta adquirir o bloqueio e `YieldProcessor` é usado enquanto a variável permanece ocupada. Cada thread realiza 10.000 aquisições; portanto, ao contrário do benchmark de mutex, o total de aquisições cresce com o número de threads.

| Cenário registrado no CSV | Threads | Média do tempo total (s) | Desvio-padrão (s) |
|---|---:|---:|---:|
| curta | 1 | 0,000199 | 0,000032 |
| curta | 2 | 0,000864 | 0,000151 |
| curta | 4 | 0,002879 | 0,000285 |
| curta | 8 | 0,006620 | 0,000393 |
| curta | 16 | 0,020063 | 0,009245 |
| media_1ms | 1 | 0,010189 | 0,000084 |
| media_1ms | 2 | 0,020614 | 0,001023 |
| media_1ms | 4 | 0,040857 | 0,000326 |
| media_1ms | 8 | 0,085488 | 0,008675 |
| media_1ms | 16 | 0,187230 | 0,026029 |
| longa_100ms | 1 | 1,001707 | 0,002692 |
| longa_100ms | 2 | 2,004866 | 0,007346 |
| longa_100ms | 4 | 4,015410 | 0,026748 |
| longa_100ms | 8 | 8,022674 | 0,018699 |
| longa_100ms | 16 | 18,973136 | 0,986174 |

O tempo aumenta de forma marcante com o número de threads, principalmente nos cenários de maior duração. Além da contenção, cada thread adicionada também aumenta o total de aquisições desta implementação. Sob espera ocupada, threads que aguardam o bloqueio continuam disputando tempo de CPU e podem atrasar a thread que está na região crítica. O desvio-padrão relativamente alto no caso de 16 threads do cenário curto e longo recomenda cautela ao interpretar esse ponto.

Os rótulos dos cenários têm a mesma divergência de unidade descrita para o mutex: os valores `1` e `100` são interpretados como microssegundos pela rotina que simula o trabalho, embora os nomes indiquem milissegundos.

### 3.3 Limitação de comparação entre os dois benchmarks Windows

Os tempos das duas tabelas não constituem, por si só, uma comparação direta entre mutex e spinlock. O benchmark de mutex executa 16.000 aquisições por rodada, independentemente do número de threads; o de spinlock executa 10.000 aquisições por thread, totalizando de 10.000 a 160.000 aquisições. Logo, as cargas diferem tanto entre mecanismos quanto conforme o número de threads. Essa diferença precisa ser considerada ao gerar gráficos ou concluir qual método é mais rápido; os códigos e dados são apresentados como encontrados, sem alteração da lógica.

## 4. Gráficos e discussão das perguntas

### Espaço reservado para os gráficos

1. **Tempo total por número de threads:** inserir gráfico com os resultados de mutex e spinlock, separado por cenário e plataforma. Identificar claramente as cargas diferentes e evitar comparar diretamente os tempos Windows até que o total de aquisições seja equivalente.

   **[Inserir gráfico: tempo total × número de threads]**

2. **Tempo de execução por duração da região crítica:** inserir gráfico com a duração efetiva no eixo horizontal e o tempo ou vazão no eixo vertical, para localizar em que duração o spinlock deixa de compensar. Usar a duração efetivamente medida, não apenas os nomes `media_1ms` e `longa_100ms`.

   **[Inserir gráfico: desempenho × duração efetiva da região crítica]**

3. **Comparação Linux × Windows / ponto de inversão:** inserir gráfico com medições equivalentes nas duas plataformas. Indicar configurações das máquinas, número de threads, carga total e dispersão das repetições.

   **[Inserir gráfico: comparação entre sistemas e ponto de inversão]**

### 4.1 Por que o tempo não diminui quando N cresce?

O incremento protegido é uma região crítica: apenas uma thread por vez pode executá-lo. Aumentar o número de threads não paraleliza esse trecho serial. Com mais threads, cresce a disputa pelo bloqueio; também pode haver custo de escalonamento, troca de contexto e tráfego de coerência entre núcleos. No spinlock, as threads em espera ainda consomem CPU tentando adquirir o bloqueio.

O efeito depende também de a carga total permanecer fixa ou crescer com N. No CSV do mutex, o total de aquisições permanece em 16.000. No CSV do spinlock, o total cresce proporcionalmente a N. Por isso, especialmente no segundo caso, o aumento do tempo não deve ser atribuído apenas à contenção.

### 4.2 Em que duração de seção crítica o spinlock deixa de compensar?

Não há um limiar universal. O spinlock tende a ser competitivo quando a espera é muito curta e o custo de suspender e despertar uma thread seria maior que o tempo gasto aguardando ativamente. Conforme a região crítica ou a espera crescem, a espera ocupada desperdiça mais CPU; o mutex tende a ser mais adequado quando pode bloquear a thread em espera.

Os resultados atuais não determinam uma duração de inversão confiável: há apenas três cenários, os valores de duração estão rotulados em unidades incompatíveis com a conversão usada no código e os benchmarks Windows têm cargas totais diferentes. Para responder numericamente com os dados do projeto, o gráfico deve usar durações efetivas variadas, uma carga equivalente para os dois mecanismos, o mesmo conjunto de números de threads e repetições suficientes para estimar a dispersão. O ponto de inversão será a duração em que o tempo (ou a vazão, conforme a métrica escolhida) do mutex passa a ser melhor que o do spinlock.

### 4.3 O ponto de inversão muda entre Linux e Windows? Que característica do SO explica a diferença?

Os dados disponíveis ainda não permitem afirmar se o ponto de inversão muda entre os sistemas. A medição Linux registrada é uma execução preliminar com quatro threads e 400.000 incrementos; os CSVs Windows variam o número de threads e usam cargas diferentes para mutex e spinlock. As métricas e condições também não são equivalentes.

Em uma comparação controlada, diferenças podem decorrer do escalonador e da implementação nativa dos bloqueios. No Linux, `pthread_mutex` pode usar um caminho rápido em espaço de usuário e recorrer a Futex para bloquear e despertar threads quando necessário; `pthread_spinlock` mantém espera ativa. No Windows, `CRITICAL_SECTION` é um bloqueio de usuário que pode combinar espera ativa e bloqueio, enquanto o spinlock deste projeto usa operações atômicas e uma instrução de pausa do processador (`YieldProcessor`), não uma suspensão da thread. Esses comportamentos, junto à quantidade de núcleos e às políticas de escalonamento, podem deslocar o ponto de equilíbrio.

Para responder à pergunta experimentalmente, é necessário executar cargas equivalentes nos dois sistemas e registrar as configurações completas. Os resultados Linux atuais e os CSVs Windows servem como evidências preliminares, não como uma demonstração de que o ponto de inversão é igual ou diferente.
