#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

#define MASTER 0
#define TAG 0
#define FIM -1

int main(int argc, char** argv) {
    int rank, nprocs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc < 2) {
        if (rank == MASTER)
            fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]);
    int token;

    if (rank == MASTER) {
        token = 0;
        printf("Processo %d: %d\n", rank, token);
        fflush(stdout);
        usleep(10000);
        token = 1;
        MPI_Send(&token, 1, MPI_INT, (MASTER + 1) % nprocs, TAG, MPI_COMM_WORLD);
    }

    while (1) {
        MPI_Recv(&token, 1, MPI_INT, (rank - 1 + nprocs) % nprocs, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (token == FIM) {
            if (rank != MASTER)
                MPI_Send(&token, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
            break;
        }

        if (token > N) {
            int fim = FIM;
            MPI_Send(&fim, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
            break;
        }

        printf("Processo %d: %d\n", rank, token);
        fflush(stdout);
        usleep(10000);

        token++;
        MPI_Send(&token, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}