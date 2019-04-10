/*
Программа считывает из файла 2 числа в 1024 знаков, записывая их в два char-овых массива длинной по 512 байт. числа динамически разбиваются на указанное число процессов и складываются
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

typedef struct send{
    char* num1;
    char* num2;
} snd;

typedef struct data_list{
    num* content;
    struct data_list* next;
} node;


void add_end(node* data, num* content){ //addingto queue element 
    while(data->next)
        data = data->next;
    data->next = malloc(sizeof(node));
    data->next->content = content;
    data->next->next=NULL;
}

num* get_next(node** data){ //getting next element from queue
    if((*data)->next)
        *data = (*data)->next;
    else return NULL;
    return (*data)->content;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv); 

    int procrang, commsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;

    FILE* inf = fopen("in.txt", "rb");
    FILE* outf = fopen("out.txt", "w");

    char tnum1[1025];
    char tnum2[1026];
    char answer[1026];

    int t1;

    char _mnum1[9];
    char _mnum2[9];

    int sum;
    char _sum[9];
    int add = 0;

    int sum1;
    char _sum1[9];
    int add1 = 1;

    node* data = malloc(sizeof(node));
    data->next = NULL;
    data->content = NULL;

    num* res;


    char send[18];
    int interact = 0;
    int adress;
    int procmap[commsize];
    for(int i=0; i<commsize; i++){
        procmap[i]=0;
    }
    int real_add = 0;
    int sequence[1024/8+1];
    char rsum[9];

    if(procrang == 0){
        fread(tnum1, 1025, 1, inf);
        fread(tnum2, 1024, 1, inf);
        tnum1[1024] = '\0';
        tnum2[1024] = '\0';

        t1 = MPI_Wtime();
        for(int a = 1024/8; a > 0; a--){
            for(int b=0; b<commsize; b++){
                procmap[b]=0;
            }
            memcpy(_mnum1, (tnum1+(a-1)*8), 8); //copying a part of number to message buffer
            memcpy(_mnum2, (tnum2+(a-1)*8), 8);
            _mnum1[8] = '\0';
            _mnum2[8] = '\0';
            memset(send, '\0', 18);
            memcpy(send, _mnum1, 9);
            memcpy((send+9), _mnum2, 9);

            MPI_Bcast(&interact, 1, MPI_INT, 0, MPI_COMM_WORLD ); //establishing connection and sending a part of number to first free process
            MPI_Recv(&interact, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            adress = status.MPI_SOURCE;
            procmap[adress] = 1;
            MPI_Scatter(procmap, 1, MPI_INT, &interact, 1, MPI_INT, 0, MPI_COMM_WORLD); //giving the task to the process that answered first
            MPI_Send(send, 18, MPI_CHARACTER, adress, adress, MPI_COMM_WORLD);
            sequence[a] = adress; //memorizing the process order
        }
        interact = -1;
        MPI_Bcast(&interact, 1, MPI_INT, 0, MPI_COMM_WORLD ); //telling all free processess there will be no job 

        add = 0;
        MPI_Barrier(MPI_COMM_WORLD);
        for(int a = 1024/8; a > 0; a--){ //from the last part to first building a sequence of main and add parts
            fflush(stdout);
            MPI_Send(&add, 1, MPI_INT, sequence[a], 100+sequence[a], MPI_COMM_WORLD);
            MPI_Recv(&add, 1, MPI_INT, sequence[a], 100+sequence[a], MPI_COMM_WORLD, &status);
            MPI_Recv(&rsum, 9, MPI_CHARACTER, sequence[a], 100+sequence[a], MPI_COMM_WORLD, &status);
            if(a>1) interact = 1;
            else interact = 0;
            MPI_Send(&interact, 1, MPI_INT, sequence[a], 100+sequence[a], MPI_COMM_WORLD);
            memcpy((answer+(a-1)*8), rsum, 8);
        }
        printf("Time: %f",MPI_Wtime()-t1);
        answer[1024] = '\0';
        fwrite(answer, 1024, 1, outf);
        fflush(stdout);

    }
    else {
        while(1){
            MPI_Bcast(&interact, 1, MPI_INT, 0, MPI_COMM_WORLD ); //establishing contact with main thread
            if(interact == -1) break;
            MPI_Send(&procrang, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Scatter(procmap, 1, MPI_INT, &interact, 1, MPI_INT, 0, MPI_COMM_WORLD);
            fflush(stdout);
            if(interact == 0) continue; //thread was not chosen, going back to wait for new request
            if(interact == -1) break; //all jobs are done now, skipping to the reporting part
            MPI_Recv(&send, 18, MPI_CHARACTER, 0, procrang, MPI_COMM_WORLD, &status);

            memcpy(_mnum1, send, 9); //making counting for the case that 1 is added in the begining and the case it is not
            memcpy(_mnum2, (send+9), 9);
            fflush(stdout);
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

            res = malloc(sizeof(num)); //storing the results
            res->sum[0] = malloc((9)*sizeof(char));
            res->sum[1] = malloc((9)*sizeof(char));
            res->add[0] = 0;
            res->add[1] = 0;
            memcpy((res->sum[0]), _sum, 9);
            memcpy((res->sum[1]), _sum1, 9);
            res->add[0] = add;
            res->add[1] = add1;

            add_end(data, res); //adding our data to queue
        }

        MPI_Barrier(MPI_COMM_WORLD);
        while(data){ // giving the requested data
            res = get_next(&data);
            if(!res) break;
            fflush(stdout);
            MPI_Recv(&real_add, 1, MPI_INT, 0, 100+procrang, MPI_COMM_WORLD, &status);
            MPI_Send(&res->add[real_add], 1, MPI_INT, 0, 100+procrang, MPI_COMM_WORLD);
            MPI_Send(res->sum[real_add], 9, MPI_CHARACTER, 0, 100+procrang, MPI_COMM_WORLD);
            MPI_Recv(&interact, 1, MPI_INT, 0, 100+procrang, MPI_COMM_WORLD, &status);
        }
    }
    MPI_Finalize();
}