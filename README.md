# FGA0244 — Programação para Sistemas Paralelos e Distribuídos

## T02 (2026.1) — Contagem Sequencial com MPI em Cluster

> **Disciplina:** FGA0244 - Programação para Sistemas Paralelos e Distribuídos  
> **Professor:** Francisco W. Cruz — fwcruz@unb.br  
> **Entrega:** Quarta-feira, 08/04/2026  

---

## 👥 Integrantes do Grupo

| Nome | Matrícula |
|------|-----------|
| Daniel dos Santos Barros de Sousa | 211030980 |
| Gabriel Freitas Balbino | 180075462 |
| Milena Baruc Rodrigues Morais | 211062339 |
| Pedro Fonseca Cruz | 212005444 |

---

## 📋 Descrição do Exercício

Implementação de um programa **MPI (Message Passing Interface)** que realiza contagem sequencial de números de `0` até `N`, onde:

- Cada processo imprime **um número por vez**
- Os processos se organizam para imprimir em **ordem crescente (0, 1, 2, ..., N)**
- A quantidade de processos é **aleatória**, mas sempre **menor que N**
- Os processos residem em **hosts distintos** (cluster MPI real)

---

## 💡 Solução — Token Ring

A solução utiliza o padrão **token ring**: os processos formam um anel e passam um token entre si. Só quem tem o token pode imprimir. Isso garante que apenas um processo imprime por vez e que a ordem é sempre sequencial.

```
node0 → milena → daniel → gabriel → pedro → node0 → ...
  0         1        2        3        4       5    → ...
```

---

## 💻 Código-Fonte

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

#define MASTER 0  // processo 0 é o chefe, ele começa tudo
#define TAG 0     // tag das mensagens, uso 0 porque só tenho um tipo de mensagem
#define FIM -1    // uso -1 como sinal pra avisar todo mundo que acabou

int main(int argc, char** argv) {
    int rank, nprocs;
    char hostname[256];
    int hostname_len;

    // inicializo o MPI e descubro quem eu sou (rank) e quantos somos (nprocs)
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Get_processor_name(hostname, &hostname_len);

    // se não passar N como argumento, aviso e encerro
    if (argc < 2) {
        if (rank == MASTER)
            fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]); // N é o número até onde vou contar
    int token;             // o token é o número que circula entre os processos

    // só o processo 0 começa — ele imprime o 0 e manda o token pra frente
    if (rank == MASTER) {
        token = 0;
        printf("Processo %d (host: %s): %d\n", rank, hostname, token);
        fflush(stdout);
        usleep(10000);
        token = 1; // incrementa antes de mandar pro próximo
        MPI_Send(&token, 1, MPI_INT, (MASTER + 1) % nprocs, TAG, MPI_COMM_WORLD);
    }

    // todos os processos (inclusive o 0) ficam nesse loop esperando o token
    while (1) {
        // cada processo espera receber do seu vizinho anterior no anel
        MPI_Recv(&token, 1, MPI_INT, (rank - 1 + nprocs) % nprocs, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // se recebi o sinal de fim, repasso pra frente (exceto o MASTER que fecha o anel)
        if (token == FIM) {
            if (rank != MASTER)
                MPI_Send(&token, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
            break;
        }

        // se o token passou de N, mando FIM pro próximo e paro
        if (token > N) {
            int fim = FIM;
            MPI_Send(&fim, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
            break;
        }

        // se ainda estou dentro do intervalo, imprimo e passo o token adiante
        printf("Processo %d (host: %s): %d\n", rank, hostname, token);
        fflush(stdout);
        usleep(10000); // pequena pausa pra saída sair na ordem certa no terminal
        token++;
        MPI_Send(&token, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
    }

    // encerro o MPI certinho
    MPI_Finalize();
    return 0;
}
```

---

## 📁 Estrutura do Repositório

```
.
├── README.md
├── src/
│   └── contagem_mpi.c    # código-fonte principal
└── scripts/
    └── hostfile           # lista de hosts do cluster
```

---

## 🛠️ Pré-requisitos

```bash
# instalar OpenMPI no Ubuntu
sudo apt install -y openmpi-bin libopenmpi-dev

# verificar instalação
mpicc --version
mpirun --version
```

---

## ⚙️ Compilação e Execução

### Local (teste rápido)

```bash
# compilar
mpicc -o contagem src/contagem_mpi.c

# rodar com 3 processos, N=15
mpirun -np 3 ./contagem 15
```

### No Cluster

```bash
# compilar
mpicc -o contagem contagem_mpi.c

# rodar com hostfile
mpirun --hostfile hostfile -np 5 /home/ubuntu/contagem 20
```

---

## 🖥️ Montagem do Cluster MPI com Multipass

Usamos o **Multipass** para criar 5 máquinas virtuais Ubuntu no Mac, simulando um cluster real com hosts distintos.

### 1. Instalar o Multipass

```bash
brew install --cask multipass
```

### 2. Criar as VMs

```bash
multipass launch --name node0   --cpus 1 --memory 512M
multipass launch --name milena  --cpus 1 --memory 512M
multipass launch --name daniel  --cpus 1 --memory 512M
multipass launch --name gabriel --cpus 1 --memory 512M
multipass launch --name pedro   --cpus 1 --memory 512M
```

### 3. Verificar os IPs

```bash
multipass list
```

| VM | IP |
|----|----|
| node0 | 192.168.2.2 |
| milena | 192.168.2.3 |
| daniel | 192.168.2.4 |
| gabriel | 192.168.2.5 |
| pedro | 192.168.2.6 |

### 4. Configurar SSH sem senha

No node0, gerar a chave SSH:

```bash
multipass shell node0
ssh-keygen -t rsa -b 4096 -f ~/.ssh/id_rsa -N ""
cat ~/.ssh/id_rsa.pub
```

Copiar a chave para as outras VMs (no terminal do Mac):

```bash
multipass exec milena  -- bash -c "mkdir -p ~/.ssh && echo 'CHAVE_PUBLICA' >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys"
multipass exec daniel  -- bash -c "mkdir -p ~/.ssh && echo 'CHAVE_PUBLICA' >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys"
multipass exec gabriel -- bash -c "mkdir -p ~/.ssh && echo 'CHAVE_PUBLICA' >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys"
multipass exec pedro   -- bash -c "mkdir -p ~/.ssh && echo 'CHAVE_PUBLICA' >> ~/.ssh/authorized_keys && chmod 700 ~/.ssh && chmod 600 ~/.ssh/authorized_keys"
```

Testar o SSH sem senha (dentro do node0):

```bash
ssh ubuntu@192.168.2.3 hostname  # milena
ssh ubuntu@192.168.2.4 hostname  # daniel
ssh ubuntu@192.168.2.5 hostname  # gabriel
ssh ubuntu@192.168.2.6 hostname  # pedro
```

### 5. Instalar OpenMPI em todas as VMs

```bash
multipass exec node0   -- sudo apt install -y openmpi-bin libopenmpi-dev
multipass exec milena  -- sudo apt install -y openmpi-bin libopenmpi-dev
multipass exec daniel  -- sudo apt install -y openmpi-bin libopenmpi-dev
multipass exec gabriel -- sudo apt install -y openmpi-bin libopenmpi-dev
multipass exec pedro   -- sudo apt install -y openmpi-bin libopenmpi-dev
```

### 6. Criar o hostfile

Dentro do node0:

```bash
cat > ~/hostfile << 'EOF'
192.168.2.2 slots=1
192.168.2.3 slots=1
192.168.2.4 slots=1
192.168.2.5 slots=1
192.168.2.6 slots=1
EOF
```

### 7. Compilar e distribuir o binário

```bash
mpicc -o contagem contagem_mpi.c
scp contagem ubuntu@192.168.2.3:/home/ubuntu/contagem
scp contagem ubuntu@192.168.2.4:/home/ubuntu/contagem
scp contagem ubuntu@192.168.2.5:/home/ubuntu/contagem
scp contagem ubuntu@192.168.2.6:/home/ubuntu/contagem
```

---

## 🔄 Saída Esperada

```bash
mpirun --hostfile hostfile -np 5 /home/ubuntu/contagem 20
```

```
Processo 0 (host: node0)   : 0
Processo 1 (host: milena)  : 1
Processo 2 (host: daniel)  : 2
Processo 3 (host: gabriel) : 3
Processo 4 (host: pedro)   : 4
Processo 0 (host: node0)   : 5
Processo 1 (host: milena)  : 6
Processo 2 (host: daniel)  : 7
Processo 3 (host: gabriel) : 8
Processo 4 (host: pedro)   : 9
Processo 0 (host: node0)   : 10
Processo 1 (host: milena)  : 11
Processo 2 (host: daniel)  : 12
Processo 3 (host: gabriel) : 13
Processo 4 (host: pedro)   : 14
Processo 0 (host: node0)   : 15
Processo 1 (host: milena)  : 16
Processo 2 (host: daniel)  : 17
Processo 3 (host: gabriel) : 18
Processo 4 (host: pedro)   : 19
Processo 0 (host: node0)   : 20
```

---

## 📚 Referências

- [OpenMPI Documentation](https://www.open-mpi.org/doc/)
- [MPI Tutorial — LLNL](https://hpc-tutorials.llnl.gov/mpi/)
- [Multipass Documentation](https://multipass.run/docs)

---

*Universidade de Brasília — Campus FGA | 2026.1*
