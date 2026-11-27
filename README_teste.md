# Projeto: Threads, Sincronização e Avaliação Multiplataforma

## Objetivo do projeto

Implementar e avaliar mecanismos de sincronização entre threads em Linux e Windows, usando as APIs nativas de cada sistema operacional. O projeto compara Mutex e Spinlock em uma região crítica que atualiza um contador compartilhado, verificando a correção do resultado e observando os efeitos da contenção, do número de threads e da duração da seção crítica.

### Resultados pretendidos

- Demonstrar exclusão mútua e evitar condições de corrida na atualização do contador.
- Apresentar implementações ou demonstrações com Mutex e Spinlock para os dois sistemas operacionais.
- Coletar tempos e, quando aplicável, dados de uso de CPU e chamadas de sincronização.
- Comparar os métodos com cargas e configurações documentadas.
- Discutir por que o tempo varia com o número de threads, quando o Spinlock deixa de compensar e se o ponto de inversão difere entre Linux e Windows.

## Integrantes

- Alicia da Silva Feliciano
- Fernanda Ferreira de Melo
- Geovanna Miguel Lamouni

## Roteiro e marcos

As etapas estão organizadas por dependência, não por datas. A equipe deve preencher as datas após definir a agenda e confirmar o estado de cada marco.

| Marco | Atividade e critério de conclusão | Estado observado | Data prevista/concluída |
|---|---|---|---|
| 1. Definição do escopo | Confirmar mecanismos, sistemas, métricas e perguntas de análise. | Escopo descrito nos arquivos do projeto. | [Definir] |
| 2. Implementações | Disponibilizar os mecanismos nativos para Windows e Linux. | Há uma camada Win32 em `code/` e uma demonstração POSIX em `testes/linux/`. | [Validar builds e registrar] |
| 3. Testes funcionais | Confirmar que Mutex e Spinlock preservam o contador esperado em cada ambiente. | Há testes Windows e demonstração Linux; consolidar evidências por sistema. | [Definir] |
| 4. Coleta experimental | Registrar configuração, carga, número de threads, repetições e resultados brutos. | Há CSVs Windows e resultados preliminares Linux; completar configurações e padronizar a metodologia. | [Em andamento / definir] |
| 5. Análise comparativa | Gerar gráficos e responder às perguntas do experimento usando dados comparáveis. | A análise está preparada no relatório; gráficos e comparação controlada ainda precisam ser concluídos. | [Definir] |
| 6. Entrega e demonstração | Revisar documentação, organizar evidências e preparar a apresentação. | Estrutura de pastas inclui `video/`; confirmar os materiais finais. | [Definir] |

## Principais entregáveis

| Entregável | Descrição | Local |
|---|---|---|
| Camada Windows de threads e sincronização | Abstração com criação/junção de threads, Mutex baseado em `CRITICAL_SECTION` e Spinlock atômico Win32. | `code/` |
| Testes funcionais Windows | Programas para conferir o valor final do contador com os mecanismos implementados. | `testes/` |
| Demonstração Linux | Programa POSIX com `pthread_mutex` e `pthread_spinlock`, além de instruções de compilação e execução. | `testes/linux/` |
| Experimentos Windows | Programas que executam rodadas com diferentes números de threads e cenários de seção crítica. | `experimentos/` |
| Dados experimentais | Resultados brutos e estatísticas Windows, além de logs Futex Linux. | `dados/` |
| Relatório | Contexto, ambientes, implementação, resultados, limitações, espaços para gráficos e discussão das perguntas. | `relatorio.md` |
| Demonstração/apresentação | Material audiovisual do projeto, se exigido na entrega. | `video/` |

## Dependências entre atividades

- Os testes funcionais dependem das implementações e do compilador disponível em cada sistema
- A coleta depende de os testes funcionais passarem e de a configuração de cada máquina estar registrada
- A comparação entre métodos depende de cargas equivalentes, unidades de duração verificadas e métricas coletadas sob condições documentadas.
- Os gráficos e as conclusões dependem da coleta consolidada; os dados preliminares não bastam para determinar com segurança o ponto de inversão.
- A apresentação final depende da revisão do relatório, dos resultados e das evidências de execução.

## Estrutura do repositório

- [`code/`](./code/) — camada de abstração para threads e sincronização Windows.
- [`testes/`](./testes/) — testes funcionais e demonstração Linux.
- [`experimentos/`](./experimentos/) — benchmarks Windows de Mutex e Spinlock.
- [`dados/`](./dados/) — CSVs, estatísticas e logs experimentais.
- [`video/`](./video/) — espaço destinado ao material de demonstração.
- [`relatorio.md`](./relatorio.md) — relatório técnico e análise do projeto.

