from pathlib import Path
import pandas as pd
import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
from matplotlib.ticker import EngFormatter

raiz = Path(__file__).resolve().parent.parent
destino = raiz / "experimentos/graficos/linux"

destino.mkdir(parents=True, exist_ok=True)

df = pd.read_csv(
    raiz / "dados/linux/benchmark_linux.csv"
)

# Validacao dos resultados
grupos = df.groupby(
    ["cenario", "mecanismo", "threads"]
)

if (
    len(df) != 900
    or len(grupos) != 30
    or not grupos.size().eq(30).all()
    or not df["aquisicoes"].eq(16000).all()
    or not df["tempo_s"].gt(0).all()
    or df.duplicated(
        ["cenario", "mecanismo", "threads", "repeticao"]
    ).any()
):
    raise SystemExit("Benchmark incompleto ou invalido.")

# Calculo da vazao
df["vazao"] = df["aquisicoes"] / df["tempo_s"]

# Calculo das estatisticas
estatisticas = df.groupby(
    ["cenario", "mecanismo", "threads"]
).agg(
    tempo_medio=("tempo_s", "mean"),
    tempo_desvio=("tempo_s", "std"),
    vazao_media=("vazao", "mean"),
    vazao_desvio=("vazao", "std"),
    repeticoes=("vazao", "count")
).reset_index()

estatisticas.to_csv(
    raiz / "dados/linux/estatisticas_linux.csv",
    index=False
)

cenarios = {
    "curta": "Seção Curta (≈ 0 µs)",
    "media": "Seção Média (1 µs)",
    "longa": "Seção Longa (100 µs)"
}

for cenario, titulo in cenarios.items():

    fig, ax = plt.subplots(figsize=(7, 4.5))

    for mecanismo, cor in [
        ("spinlock", "#4472C4"),
        ("mutex", "#C0504D")
    ]:

        dados = estatisticas[
            (estatisticas["cenario"] == cenario) &
            (estatisticas["mecanismo"] == mecanismo)
        ].sort_values("threads")

        ax.errorbar(
            dados["threads"],
            dados["vazao_media"],
            yerr=dados["vazao_desvio"],
            label=mecanismo.capitalize(),
            color=cor,
            marker="o",
            linewidth=1.8,
            markersize=5,
            capsize=4
        )

    ax.set_title(titulo, fontweight="bold")
    ax.set_xlabel("Número de threads (N)")
    ax.set_ylabel("Vazão (aquisições/s)")

    ax.set_xticks([1, 2, 4, 8, 16])
    ax.set_ylim(bottom=0)

    ax.yaxis.set_major_formatter(EngFormatter())

    ax.legend(loc="best")
    ax.grid(True, linestyle="--", alpha=0.3)

    fig.tight_layout()

    for extensao in ["png", "pdf", "svg"]:
        fig.savefig(
            destino / f"{cenario}.{extensao}",
            dpi=300,
            bbox_inches="tight"
        )

    plt.close(fig)

    print(f"Gráfico gerado: {cenario}.png")

print("\n===== RESULTADOS EXPERIMENTAIS =====\n")
print(estatisticas.to_string(index=False))

print("\n===== COMPARACAO DE VAZAO =====\n")

comparacao = estatisticas.pivot(
    index=["cenario", "threads"],
    columns="mecanismo",
    values="vazao_media"
).reset_index()

comparacao["melhor_mecanismo"] = comparacao.apply(
    lambda linha: "Mutex"
    if linha["mutex"] > linha["spinlock"]
    else "Spinlock"
    if linha["spinlock"] > linha["mutex"]
    else "Empate",
    axis=1
)

print(comparacao.to_string(index=False))

print("\nTodos os gráficos foram gerados com sucesso!")
