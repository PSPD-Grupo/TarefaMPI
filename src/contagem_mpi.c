#include <stdio.h>
#include <mpi.h>

#define MASTER 0
#define TAG 0

int main(int argc, char** argv) {
    int rank, nprocs;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int token = 0;

    if (rank == MASTER) {
        printf("Processo %d enviando para o processo %d\n", rank, (rank + 1) % nprocs);

        MPI_Send(&token, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
        MPI_Recv(&token, 1, MPI_INT, (rank - 1) % nprocs, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    } else {
        MPI_Recv(&token, 1, MPI_INT, (rank - 1) % nprocs, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        token++;
        printf("Processo %d enviando para o processo %d\n", rank, (rank + 1) % nprocs);

        MPI_Send(&token, 1, MPI_INT, (rank + 1) % nprocs, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}