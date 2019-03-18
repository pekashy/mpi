#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>


/*Измерить время работы 4 коллективных функций MPI: MPI_Bcast, MPI_Reduce (для любой операции и типа данных), MPI_Gather и MPI_Scatter. Добиться погрешности измерения порядка MPI_Wtick.*/

/*
Scatter is 1
Bcast is 2
Reduce is 3
Gather is 4
*/
double test(int type, int commsize){
    MPI_Status status;
    int* rbuf = malloc(sizeof(int)*10); 
    int* stbuf = malloc(commsize*10*sizeof(int));//some payload
    int r, s;
    double t2, t1;
    if(type == 1){
        t1 = MPI_Wtime();
        MPI_Scatter(stbuf,10,MPI_INT, rbuf,10,MPI_INT,0, MPI_COMM_WORLD);
        t2 = MPI_Wtime();
    }
    if(type == 2){
        for(int i=0; i<10*commsize; i++){
            stbuf[i] = i;
        }
        t1 = MPI_Wtime();
        MPI_Bcast(stbuf, 10*commsize, MPI_INT, 0, MPI_COMM_WORLD );
        t2 = MPI_Wtime();
    }
    if(type == 3){
        for(int i=0; i<10; i++){
            rbuf[i] = i;
        }
        int res=0;
        int rds = 3;
        t1 = MPI_Wtime();
        MPI_Reduce(&res, &rds, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        t2 = MPI_Wtime();
    }
    if(type == 4){
        r=1;
        t1 = MPI_Wtime();
        MPI_Gather(&r, 1, MPI_INT, stbuf, 1*commsize, MPI_INT, 0, MPI_COMM_WORLD);
        t2 = MPI_Wtime();
    }
    
    free(rbuf);
    free(stbuf);
    return t2-t1;
}



int main(int argc, char **argv){
    int type = atoi(argv[1]);
    MPI_Init(&argc, &argv); int commsize;
    double t1, t2;
    int procrang;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;

    double tick = MPI_Wtick(); //getting tick length
    if(procrang == 0)
        printf("%e\n", tick); 

    int rbuf = 0; 
    int stbuf[commsize];
    int n=1;
    double stime=0, lstime=100;
    double summ=0, lsumm=100;
    double testtime;
    double msg[2];
    double diff;
    do{
        MPI_Barrier(MPI_COMM_WORLD);
        testtime = test(type, commsize); //running test for thread
        lsumm = summ;
        MPI_Reduce(&testtime, &stime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        stime /= commsize; //getting mean time among thee threads
        MPI_Bcast(&stime, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD );//syncing mean time

        if(n>1) summ = (summ*(n-1)+stime)/n; 
        else summ = stime;//counting mean among the runs

        diff = summ-lsumm;//diff changing between means
        if(diff<0)
            diff = -diff;
            
        n++; 
    } while(diff > tick); //tick^2 for the sake of some extra precision
    if(procrang == 0)
        printf("RESULT s: %e\n", summ);
    
    MPI_Finalize();
    return(0);
}
