#!/bin/bash

set -e

clear

echo "=========================================="
echo " UNIVERSIDADE FEDERAL DE UBERLANDIA"
echo " FACOM - SISTEMAS OPERACIONAIS"
echo " TEMA 1 - MUTEX E SPINLOCK NO LINUX"
echo "=========================================="

echo
echo "===== AMBIENTE EXPERIMENTAL ====="
echo

grep PRETTY_NAME /etc/os-release

echo "Kernel: $(uname -r)"
echo "Processadores logicos: $(nproc)"

lscpu | grep "Nome do modelo"

echo
read -p "Pressione ENTER para compilar o programa..."

gcc -std=c11 -O2 -Wall -Wextra -pthread \
    demo_mutex_spinlock_linux.c -o demo

echo "Compilacao realizada com sucesso!"

echo
read -p "Pressione ENTER para executar o Mutex..."

echo
echo "========== MUTEX =========="
time ./demo mutex

echo
read -p "Pressione ENTER para executar o Spinlock..."

echo
echo "========== SPINLOCK =========="
time ./demo spin

echo
read -p "Pressione ENTER para analisar o Futex..."

echo
echo "========== FUTEX - MUTEX =========="

strace -f -c -e trace=futex ./demo mutex

echo
read -p "Pressione ENTER para continuar..."

echo
echo "========== FUTEX - SPINLOCK =========="

strace -f -c -e trace=futex ./demo spin

echo
echo "=========================================="
echo " DEMONSTRACAO FINALIZADA COM SUCESSO"
echo "=========================================="
