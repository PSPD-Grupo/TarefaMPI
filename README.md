# 🔢 FGA0244 — Programação para Sistemas Paralelos e Distribuídos

## T02 (2026.1) — Contagem Sequencial com MPI em Cluster

> **Disciplina:** FGA0244 - Programação para Sistemas Paralelos e Distribuídos  
> **Professor:** Francisco W. Cruz — fwcruz@unb.br  
> **Entrega:** Quarta-feira, 08/04/2026  
> **Grupo:** 4 alunos

---

## 📋 Descrição do Exercício

Implementação de um programa **MPI (Message Passing Interface)** que realiza contagem sequencial de números de `0` até `N`, onde:

- Cada processo imprime **um número por vez**
- Os processos se organizam para imprimir em **ordem crescente (0, 1, 2, ..., N)**
- A quantidade de processos é **aleatória**, mas sempre **menor que N**
- Os processos devem residir em **hosts distintos** (cluster MPI real)

---

## 🧠 Lógica do Programa

```
Processo 0 imprime: 0, P, 2P, ...
Processo 1 imprime: 1, P+1, 2P+1, ...
...
Processo k imprime: k, P+k, 2P+k, ...

Onde P = número de processos
```

A sincronização garante que apenas **um processo imprime por vez**, usando passagem de mensagens (token de permissão).

---

## 📁 Estrutura do Repositório

```
.
├── README.md
├── src/
│   └── contagem_mpi.c       # Código-fonte principal
├── scripts/
│   ├── setup_cluster.sh     # Script de configuração do cluster
│   ├── run.sh               # Script para execução distribuída
│   └── hostfile             # Lista de hosts do cluster
└── docs/
    └── relatorio.md         # Relatório explicando a solução
```

---

## 🛠️ Pré-requisitos

```bash
# Instalar OpenMPI no Ubuntu
sudo apt update
sudo apt install -y openmpi-bin libopenmpi-dev

# Verificar instalação
mpicc --version
mpirun --version
```

---

## ⚙️ Compilação e Execução

### Local (teste rápido)

```bash
# Compilar
mpicc -o contagem src/contagem_mpi.c

# Executar com 4 processos, N=20
mpirun -np 4 ./contagem 20
```

### Em Cluster (hosts distintos)

```bash
# Compilar em todos os nós (ou usar NFS)
mpicc -o contagem src/contagem_mpi.c

# Executar com hostfile
mpirun --hostfile scripts/hostfile -np 4 ./contagem 20
```

---

## 🖥️ Montagem do Cluster MPI no Ubuntu Linux

### 1. Configurar SSH sem senha entre os nós

```bash
# No nó mestre, gerar chave SSH
ssh-keygen -t rsa -b 4096

# Copiar chave para cada nó worker
ssh-copy-id usuario@node1
ssh-copy-id usuario@node2
# ...

# Testar acesso sem senha
ssh usuario@node1 "hostname"
```

### 2. Criar o hostfile

```bash
# scripts/hostfile
node0 slots=2   # nó mestre
node1 slots=2   # nó worker 1
node2 slots=2   # nó worker 2
```

> 💡 Use IPs ou configure o `/etc/hosts` de todos os nós para resolução de nomes.

### 3. Compartilhar o binário via NFS (recomendado)

```bash
# No nó mestre — instalar NFS server
sudo apt install nfs-kernel-server

# Exportar diretório compartilhado
echo "/home/usuario/mpi_trabalho  *(rw,sync,no_subtree_check)" | sudo tee -a /etc/exports
sudo exportfs -a
sudo systemctl restart nfs-kernel-server

# Nos nós workers — montar o diretório
sudo mount node0:/home/usuario/mpi_trabalho /home/usuario/mpi_trabalho
```

### 4. Verificar conectividade do cluster

```bash
# Testar comunicação MPI entre hosts
mpirun --hostfile scripts/hostfile -np 4 hostname
```

---

## 🔄 Exemplo de Saída

```bash
$ mpirun --hostfile scripts/hostfile -np 3 ./contagem 10

Processo 0 (host: node0): imprime 0
Processo 1 (host: node1): imprime 1
Processo 2 (host: node2): imprime 2
Processo 0 (host: node0): imprime 3
Processo 1 (host: node1): imprime 4
Processo 2 (host: node2): imprime 5
Processo 0 (host: node0): imprime 6
Processo 1 (host: node1): imprime 7
Processo 2 (host: node2): imprime 8
Processo 0 (host: node0): imprime 9
Processo 1 (host: node1): imprime 10
```

---

## 📌 Notas Importantes

- O número de processos (`-np`) deve ser **menor que N**
- A sincronização é feita via **passagem de token** (ring topology)
- Cada processo sabe seu `rank` e imprime o número atual do token
- O token circula em anel até ultrapassar N

---

## 👥 Integrantes do Grupo

| Nome | Matrícula |
|------|-----------|
| Daniel dos Santos Barros de Sousa | 211030980 |
| Gabriel Freitas Balbino | 180075462 |
| Milena Baruc Rodrigues Morais | 211062339 |
| Pedro Fonseca Cruz | 212005444 |

---

## 📚 Referências

- [OpenMPI Documentation](https://www.open-mpi.org/doc/)
- [MPI Tutorial — LLNL](https://hpc-tutorials.llnl.gov/mpi/)
- [Setting up MPI Cluster on Ubuntu](https://mpitutorial.com/tutorials/running-an-mpi-cluster-within-a-lan/)
- `man mpirun`, `man mpicc`

---

*Universidade de Brasília — Campus FGA | 2026.1*
