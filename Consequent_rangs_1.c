#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv){
    int t1, t2;
    t1=MPI_Wtime();
    MPI_Init(&argc, &argv); int commsize;
    int procrang;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;
    int buf = 0;
    if (procrang == 0){
        printf("%d %f\n", procrang, (MPI_Wtime()-t1));
        fflush(stdout);
        MPI_Send(&buf, 1, MPI_INT, procrang + 1, 0, MPI_COMM_WORLD);
    }
    else{
        MPI_Recv(&buf, 1, MPI_INT, procrang - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("%d %f\n", procrang, (MPI_Wtime()-t1));
        fflush(stdout);
        if (procrang < commsize-1)
            MPI_Send(&buf, 1, MPI_INT, procrang + 1, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return (0);
}
