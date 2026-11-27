from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Diretorio raiz do repositorio
raiz = Path(__file__).resolve().parent.parent

arquivo = raiz / "dados/linux/resultados_preliminares.csv"
destino = raiz / "experimentos/graficos/linux"

destino.mkdir(parents=True, exist_ok=True)

# Leitura dos resultados reais
df = pd.read_csv(arquivo)

# Calculo da vazao aproximada
df["vazao"] = df["incrementos"] / (df["tempo_ms"] / 1000)

# Identidade visual da apresentacao
cores = {
    "Mutex": "#38BDF8",
    "Spinlock": "#F59E0B"
}

plt.rcParams.update({
    "font.size": 11,
    "axes.titlesize": 15,
    "axes.labelsize": 12,
    "figure.dpi": 150
})

def gerar_grafico(coluna, titulo, eixo_y, nome):

    tabela = df.pivot(
        index="data",
        columns="mecanismo",
        values=coluna
    )

    fig, ax = plt.subplots(figsize=(8, 4.8))

    x = np.arange(len(tabela))
    largura = 0.32

    for i, mecanismo in enumerate(["Mutex", "Spinlock"]):

        barras = ax.bar(
            x + (i - 0.5) * largura,
            tabela[mecanismo],
            largura,
            label=mecanismo,
            color=cores[mecanismo]
        )

        ax.bar_label(barras, fmt="%.2f", padding=4)

    ax.set_title(titulo, fontweight="bold")
    ax.set_ylabel(eixo_y)
    ax.set_xlabel("Data da execução")

    ax.set_xticks(x)
    ax.set_xticklabels(["25/11/2026", "26/11/2026"])

    ax.set_ylim(0, tabela.max().max() * 1.25)

    ax.legend()
    ax.grid(axis="y", linestyle="--", alpha=0.25)
    ax.set_axisbelow(True)

    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

    fig.text(
        0.5, 0.02,
        "Fonte: dados próprios — medições preliminares no Linux.",
        ha="center",
        fontsize=9,
        color="#64748B"
    )

    plt.tight_layout(rect=[0, 0.06, 1, 1])

    fig.savefig(
        destino / f"{nome}.png",
        dpi=300,
        bbox_inches="tight"
    )

    fig.savefig(
        destino / f"{nome}.pdf",
        bbox_inches="tight"
    )

    plt.close(fig)

    print(f"Gráfico gerado: {nome}")

# Grafico 1
gerar_grafico(
    "tempo_ms",
    "Mutex vs Spinlock — Tempo de execução",
    "Tempo total (ms)",
    "01_tempo_execucao"
)

# Grafico 2
gerar_grafico(
    "futex",
    "Mutex vs Spinlock — Chamadas Futex",
    "Quantidade de chamadas",
    "02_chamadas_futex"
)

# Grafico 3
gerar_grafico(
    "user_ms",
    "Tempo de CPU em modo usuário",
    "Tempo de CPU (ms)",
    "03_tempo_usuario"
)

# Grafico 4
gerar_grafico(
    "sys_ms",
    "Tempo de CPU em modo kernel",
    "Tempo de CPU (ms)",
    "04_tempo_kernel"
)

# Grafico 5
df["vazao_milhoes"] = df["vazao"] / 1000000

gerar_grafico(
    "vazao_milhoes",
    "Mutex vs Spinlock — Vazão aproximada",
    "Milhões de operações/s",
    "05_vazao"
)

print("\nTodos os gráficos foram gerados!")
print(destino)
