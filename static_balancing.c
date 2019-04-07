/*
Программа считывает из файла 2 числа в 1024 знаков, записывая их в два char-овых массива длинной по 512 байт. Числа разбиваются на блоки (количество блоков = количеству процессов), которые рассылаются всем процессам. В каждом процессе происходит спекулятивное вычисление результата, после которого происходит пересылка следующему процессу перенос разряда. После пересылки всех переносов разряда собирается полный результат, который записывается в файл. Программа должна уметь запускаться на 2^n процессах, где n < 6. В консоль необходимо вывести время работы алгоритма (от считывания чисел и до вывода результата). 

*/
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

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

    //printf("%d sum: %s %s\n",procrang, _num1, _num2);
    fflush(stdout);
    char _mnum1[9];
    char _mnum2[9];
    int sum;
    char _sum[9];
    int add = 0;
    if(procrang != commsize-1){
        MPI_Recv(&add, 1, MPI_INT, procrang+1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

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
        //printf("    %s %s %d %d\n", _mnum1, _mnum2, sum, add);
        if(add == 1) sum-=1e8;
        if(sum<1e7) sprintf(_sum, "0%d", sum);
        else sprintf(_sum, "%d", sum);
        _sum[8] = '\0';
        //printf("sum: %s\n", _sum);
        memcpy((_gsum + (i-1)*8), _sum, 8);
    }
    
    gadd = add;
    _gsum[length] = '\0';
    //printf("%d gsum: %d %s\n", procrang, gadd, _gsum);
    if(procrang>0){
        MPI_Send(&gadd, 1, MPI_INT, procrang-1, 0, MPI_COMM_WORLD);
        MPI_Send(_gsum, length+1, MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
    }
    else{    
        if(gadd == 1) fwrite("1", 1, 1, outf);

        //printf("0: %s\n", _gsum);
        fwrite(_gsum, length, 1, outf);
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