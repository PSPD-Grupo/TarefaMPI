#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

#define MASTER 0  // processo 0 é o chefe, ele começa tudo
#define TAG 0     // tag das mensagens, uso 0 porque só tenho um tipo de mensagem
#define FIM -1    // uso -1 como sinal pra avisar todo mundo que acabou

int main(int argc, char** argv) {
    int rank, nprocs;
    int hostname_len;
    char hostname[256];

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
        printf("Processo %d (host: %s) : %d\n", rank, hostname, token);   
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
        printf("Processo %d (host: %s) : %d\n", rank, hostname, token);
        fflush(stdout);
        usleep(10000); // pequena pausa pra saída sair na ordem certa no terminal
        token++;
        MPI_Send(&token, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
    }

    // encerro o MPI certinho
    MPI_Finalize();
    return 0;
}
