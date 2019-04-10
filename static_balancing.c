/*
Программа считывает из файла 2 числа в 1024 знаков, записывая их в два char-овых массива длинной по 512 байт. Числа разбиваются на блоки (количество блоков = количеству процессов), которые рассылаются всем процессам. В каждом процессе происходит спекулятивное вычисление результата, после которого происходит пересылка следующему процессу перенос разряда. После пересылки всех переносов разряда собирается полный результат, который записывается в файл. Программа должна уметь запускаться на 2^n процессах, где n < 6. В консоль необходимо вывести время работы алгоритма (от считывания чисел и до вывода результата). 

*/
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef struct number{
    char* sum[2];
    int add[2];
} num;


int main(int argc, char** argv){
    MPI_Init(&argc, &argv); 

    int procrang, commsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;

    int ppow = atoi(argv[1]);
    int pnum = (int) pow(2.0, (double) ppow);
    int length = 1024/pnum;
    FILE* inf = fopen("in.txt", "rb");
    FILE* outf = fopen("out.txt", "w");

    char tnum1[1025];
    char tnum2[1026];
    
    fread(tnum1, 1025, 1, inf);
    fread(tnum2, 1024, 1, inf);
    tnum1[1024] = '\0';
    tnum2[1024] = '\0';
    int t1 = MPI_Wtime();

    char _num1[length+1];
    char _num2[length+1];
    strncpy(_num1, (tnum1+procrang*length), length);
    strncpy(_num2, (tnum2+procrang*length), length);
    _num1[length] = '\0';
    _num2[length] = '\0';

    char _mnum1[9];
    char _mnum2[9];
    
    int sum;
    char _sum[9];
    int add = 0;

    int sum1;
    char _sum1[9];
    int add1 = 1;

    num* res = malloc(sizeof(num));
    res->sum[0] = malloc((length+1)*sizeof(char));
    res->sum[1] = malloc((length+1)*sizeof(char));
    res->add[0] = 0;
    res->add[1] = 0;

    char* _gsum = malloc(sizeof(char)*(length+1));
    char gadd = 0;
    
    for(int i = length/8; i > 0; i--){
        //printf("%d %d\n",i,  length/8);
        memcpy(_mnum1, (_num1+(i-1)*8), 8);
        memcpy(_mnum2, (_num2+(i-1)*8), 8);
        _mnum1[8] = '\0';
        _mnum2[8] = '\0';

        sum = atoi(_mnum1) + atoi(_mnum2) + add;
        add = sum/1e8;
        if(add == 1) sum-=1e8;
        if(sum<1e7) sprintf(_sum, "0%d", sum);
        else sprintf(_sum, "%d", sum);
        _sum[8] = '\0';

        sum1 = atoi(_mnum1) + atoi(_mnum2) + add1;
        add1 = sum1/1e8;
        if(add1 == 1) sum1-=1e8;
        if(sum1<1e7) sprintf(_sum1, "0%d", sum1);
        else sprintf(_sum1, "%d", sum1);
        _sum1[8] = '\0';

        memcpy((res->sum[0] + (i-1)*8), _sum, 8);
        memcpy((res->sum[1] + (i-1)*8), _sum1, 8);
    }
    
    res->add[0] = add;
    res->add[1] = add1;
    res->sum[0][length] = '\0';
    res->sum[1][length] = '\0';

    //printf("%d gsum: %d %s\n", procrang, gadd, _gsum);
    int real_add = 0;
    if(procrang != commsize-1){
        MPI_Recv(&real_add, 1, MPI_INT, procrang+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
    
    if(procrang>0){
        MPI_Send(&res->add[real_add], 1, MPI_INT, procrang-1, 0, MPI_COMM_WORLD);
        MPI_Send(res->sum[real_add], length+1, MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
    }
    else{    
        if(res->add[real_add] == 1) fwrite("1", 1, 1, outf);
        //printf("0: %s\n", _gsum);
        fwrite(res->sum[real_add], length, 1, outf);
        fflush(stdout);
        char rsum[length+1];
        for(int i=1; i < pnum; i++){
            MPI_Recv(&rsum, length+1, MPI_CHARACTER, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //printf("%d: %s\n", i, rsum);
            fwrite(rsum, length, 1, outf);
        }
        printf("Calculation time: %f", MPI_Wtime()-t1);
    }
    MPI_Finalize();
}