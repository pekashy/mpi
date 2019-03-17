#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv){
    MPI_Init(&argc, &argv); int commsize;
    int t1, t2;
    t1=MPI_Wtime();
    int procrang;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;
    int buf = 0;
    if (procrang == 0){
        printf("%d %f\n", procrang, (MPI_Wtime()-t1));
        fflush(stdout);
        for(int a=0; a<commsize; a++){
            MPI_Send(&buf, 1, MPI_INT, a, 0, MPI_COMM_WORLD);
            MPI_Recv(&buf, 1, MPI_INT, a, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }
    else{
        MPI_Recv(&buf, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("%d %f\n", procrang, (MPI_Wtime()-t1));
        fflush(stdout);
        MPI_Send(&buf, 1, MPI_INT, 0, procrang, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return (0);
}
