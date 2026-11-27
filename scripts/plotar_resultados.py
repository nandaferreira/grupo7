#!/usr/bin/env python3
"""Gera gráficos SVG de vazão, sem dependências além da biblioteca padrão.

Uso:
    python scripts/plotar_resultados.py
    python scripts/plotar_resultados.py --saida dados/meu_grafico.svg

O SVG pode ser aberto no navegador. As barras indicam um desvio-padrão.
"""

import argparse
import csv
import html
import math
import statistics
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CORES = {"Spinlock": "#2563eb", "Mutex": "#dc2626"}
LARGURA_PAINEL = 600
ALTURA = 500


def carregar_medias(caminho):
    """Agrupa as vazões por cenário e número de threads."""
    valores = {}

    with caminho.open("r", newline="", encoding="utf-8-sig") as arquivo:
        leitor = csv.DictReader(arquivo)
        if leitor.fieldnames is None:
            raise ValueError(f"CSV sem cabeçalho: {caminho}")

        leitor.fieldnames = [nome.strip() for nome in leitor.fieldnames]
        obrigatorias = {"cenario", "num_threads", "vazao"}
        faltantes = obrigatorias.difference(leitor.fieldnames)
        if faltantes:
            nomes = ", ".join(sorted(faltantes))
            raise ValueError(f"Colunas ausentes em {caminho}: {nomes}")

        for numero_linha, linha in enumerate(leitor, start=2):
            try:
                cenario = linha["cenario"].strip()
                threads = int(linha["num_threads"])
                vazao = float(linha["vazao"])
            except (AttributeError, TypeError, ValueError) as erro:
                raise ValueError(
                    f"Dados inválidos em {caminho}, linha {numero_linha}"
                ) from erro

            if not cenario or threads <= 0 or not math.isfinite(vazao) or vazao <= 0:
                raise ValueError(
                    f"Dados inválidos em {caminho}, linha {numero_linha}"
                )
            valores.setdefault((cenario, threads), []).append(vazao)

    if not valores:
        raise ValueError(f"Nenhum dado encontrado em {caminho}")

    medias = {}
    for (cenario, threads), amostras in valores.items():
        desvio = statistics.stdev(amostras) if len(amostras) > 1 else 0.0
        medias.setdefault(cenario, {})[threads] = (
            statistics.mean(amostras),
            desvio,
        )
    return medias


def encontrar_cruzamentos(spinlock, mutex):
    """Interpola cruzamentos entre os pontos medidos consecutivos."""
    cruzamentos = []
    threads = sorted(spinlock)
    diferencas = {
        thread: spinlock[thread][0] - mutex[thread][0] for thread in threads
    }

    for thread in threads:
        if diferencas[thread] == 0:
            cruzamentos.append((float(thread), spinlock[thread][0]))

    for anterior, atual in zip(threads, threads[1:]):
        diferenca_anterior = diferencas[anterior]
        diferenca_atual = diferencas[atual]
        if diferenca_anterior * diferenca_atual < 0:
            proporcao = -diferenca_anterior / (
                diferenca_atual - diferenca_anterior
            )
            thread_cruzamento = anterior + proporcao * (atual - anterior)
            vazao_cruzamento = spinlock[anterior][0] + proporcao * (
                spinlock[atual][0] - spinlock[anterior][0]
            )
            cruzamentos.append((thread_cruzamento, vazao_cruzamento))

    return sorted(cruzamentos)


def validar_dados(spinlock, mutex):
    if spinlock.keys() != mutex.keys():
        raise ValueError("Os CSVs não contêm os mesmos cenários.")

    for cenario in spinlock:
        if spinlock[cenario].keys() != mutex[cenario].keys():
            raise ValueError(
                f"Os CSVs não contêm os mesmos números de threads "
                f"no cenário '{cenario}'."
            )


def formatar_vazao(valor):
    if valor >= 1_000_000:
        return f"{valor / 1_000_000:.1f}M"
    if valor >= 1_000:
        return f"{valor / 1_000:.1f}k"
    return f"{valor:.1f}"


def adicionar_texto(elementos, x, y, texto, **atributos):
    opcoes = {"font-size": "13", "fill": "#374151"}
    opcoes.update(atributos)
    atributos_svg = " ".join(
        f'{chave.replace("_", "-")}="{html.escape(str(valor), quote=True)}"'
        for chave, valor in opcoes.items()
    )
    elementos.append(
        f'<text x="{x:.2f}" y="{y:.2f}" {atributos_svg}>'
        f"{html.escape(str(texto))}</text>"
    )


def adicionar_painel(elementos, cenario, spinlock, mutex, deslocamento):
    esquerda = deslocamento + 75
    topo = 90
    largura_grafico = 490
    altura_grafico = 310
    threads = sorted(spinlock[cenario])
    menor_thread, maior_thread = threads[0], threads[-1]
    maior_vazao = max(
        media + desvio
        for dados in (spinlock[cenario], mutex[cenario])
        for media, desvio in dados.values()
    )
    limite_y = maior_vazao * 1.08

    def pos_x(thread):
        if menor_thread == maior_thread:
            return esquerda + largura_grafico / 2
        return esquerda + (thread - menor_thread) / (
            maior_thread - menor_thread
        ) * largura_grafico

    def pos_y(vazao):
        return topo + altura_grafico - vazao / limite_y * altura_grafico

    adicionar_texto(
        elementos,
        deslocamento + LARGURA_PAINEL / 2,
        45,
        cenario,
        **{"font-size": "19", "font-weight": "bold", "text-anchor": "middle"},
    )

    for passo in range(6):
        valor = limite_y * passo / 5
        y = pos_y(valor)
        elementos.append(
            f'<line x1="{esquerda}" y1="{y:.2f}" '
            f'x2="{esquerda + largura_grafico}" y2="{y:.2f}" '
            'stroke="#e5e7eb" />'
        )
        adicionar_texto(
            elementos,
            esquerda - 9,
            y + 4,
            formatar_vazao(valor),
            **{"text-anchor": "end"},
        )

    for thread in threads:
        x = pos_x(thread)
        elementos.append(
            f'<line x1="{x:.2f}" y1="{topo}" x2="{x:.2f}" '
            f'y2="{topo + altura_grafico}" stroke="#f3f4f6" />'
        )
        adicionar_texto(
            elementos,
            x,
            topo + altura_grafico + 22,
            thread,
            **{"text-anchor": "middle"},
        )

    elementos.extend(
        [
            f'<line x1="{esquerda}" y1="{topo}" x2="{esquerda}" '
            f'y2="{topo + altura_grafico}" stroke="#374151" />',
            f'<line x1="{esquerda}" y1="{topo + altura_grafico}" '
            f'x2="{esquerda + largura_grafico}" '
            f'y2="{topo + altura_grafico}" stroke="#374151" />',
        ]
    )
    adicionar_texto(
        elementos,
        esquerda + largura_grafico / 2,
        topo + altura_grafico + 49,
        "Número de threads",
        **{"text-anchor": "middle"},
    )
    adicionar_texto(
        elementos,
        deslocamento + 16,
        topo + altura_grafico / 2,
        "Vazão (aquisições/s)",
        transform=(
            f"rotate(-90 {deslocamento + 16} "
            f"{topo + altura_grafico / 2})"
        ),
        **{"text-anchor": "middle"},
    )

    for nome, dados in (
        ("Spinlock", spinlock[cenario]),
        ("Mutex", mutex[cenario]),
    ):
        pontos = [
            (pos_x(thread), pos_y(dados[thread][0])) for thread in threads
        ]
        coordenadas = " ".join(f"{x:.2f},{y:.2f}" for x, y in pontos)
        elementos.append(
            f'<polyline points="{coordenadas}" fill="none" '
            f'stroke="{CORES[nome]}" stroke-width="2.5" />'
        )

        for thread, (media, desvio) in dados.items():
            x = pos_x(thread)
            y = pos_y(media)
            y_superior = pos_y(media + desvio)
            y_inferior = pos_y(max(0, media - desvio))
            elementos.extend(
                [
                    f'<line x1="{x:.2f}" y1="{y_superior:.2f}" '
                    f'x2="{x:.2f}" y2="{y_inferior:.2f}" '
                    f'stroke="{CORES[nome]}" />',
                    f'<line x1="{x - 4:.2f}" y1="{y_superior:.2f}" '
                    f'x2="{x + 4:.2f}" y2="{y_superior:.2f}" '
                    f'stroke="{CORES[nome]}" />',
                    f'<line x1="{x - 4:.2f}" y1="{y_inferior:.2f}" '
                    f'x2="{x + 4:.2f}" y2="{y_inferior:.2f}" '
                    f'stroke="{CORES[nome]}" />',
                    f'<circle cx="{x:.2f}" cy="{y:.2f}" r="4" '
                    f'fill="{CORES[nome]}" />',
                ]
            )

    for indice, nome in enumerate(CORES):
        x = deslocamento + 205 + indice * 140
        elementos.append(
            f'<line x1="{x}" y1="74" x2="{x + 24}" y2="74" '
            f'stroke="{CORES[nome]}" stroke-width="3" />'
        )
        adicionar_texto(elementos, x + 31, 79, nome)

    cruzamentos = encontrar_cruzamentos(spinlock[cenario], mutex[cenario])
    for thread, vazao in cruzamentos:
        x, y = pos_x(thread), pos_y(vazao)
        elementos.append(
            f'<circle cx="{x:.2f}" cy="{y:.2f}" r="7" fill="#111827" />'
        )
        print(
            f"{cenario}: cruzamento estimado em {thread:.2f} threads "
            f"(vazão aprox. {vazao:,.2f} aquisições/s)."
        )
    if not cruzamentos:
        print(
            f"{cenario}: não há cruzamento entre os pontos medidos "
            "nem entre os segmentos interpolados."
        )


def gerar_svg(spinlock, mutex, saida):
    cenarios = list(spinlock)
    largura = LARGURA_PAINEL * len(cenarios)
    elementos = [
        f'<svg xmlns="http://www.w3.org/2000/svg" '
        f'width="{largura}" height="{ALTURA}" '
        f'viewBox="0 0 {largura} {ALTURA}">',
        "<title>Vazão por número de threads</title>",
        (
            "<desc>Médias e desvios-padrão de vazão para mutex e spinlock, "
            "separados por cenário.</desc>"
        ),
        '<rect width="100%" height="100%" fill="white" />',
    ]

    for indice, cenario in enumerate(cenarios):
        adicionar_painel(
            elementos,
            cenario,
            spinlock,
            mutex,
            indice * LARGURA_PAINEL,
        )

    elementos.append("</svg>")
    saida.parent.mkdir(parents=True, exist_ok=True)
    saida.write_text("\n".join(elementos), encoding="utf-8")
    print(f"Gráfico salvo em: {saida}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--spinlock",
        type=Path,
        default=ROOT / "dados" / "dados_brutos_spinlock.csv",
        help="CSV bruto do spinlock.",
    )
    parser.add_argument(
        "--mutex",
        type=Path,
        default=ROOT / "dados" / "dados_brutos_mutex.csv",
        help="CSV bruto do mutex.",
    )
    parser.add_argument(
        "--saida",
        type=Path,
        default=ROOT / "dados" / "graficos_vazao.svg",
        help="Caminho do gráfico SVG gerado.",
    )
    argumentos = parser.parse_args()

    spinlock = carregar_medias(argumentos.spinlock)
    mutex = carregar_medias(argumentos.mutex)
    validar_dados(spinlock, mutex)
    gerar_svg(spinlock, mutex, argumentos.saida)


if __name__ == "__main__":
    main()
