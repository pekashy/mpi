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

typedef struct send{
    char* num1;
    char* num2;
} snd;

typedef struct data_list{
    num* content;
    struct data_list* next;
} node;


void add_end(node* data, num* content){
    /*if(!data->content){
        data->content = content;
        data->next = NULL;
        return;
    }*/
    while(data->next)
        data = data->next;
    data->next = malloc(sizeof(node));
    data->next->content = content;
    data->next->next=NULL;
}

void delete_last(node* data){
    if(data->next){
        while(data->next->next)
            data = data->next;
        if(data->next->content){
            if(data->next->content->sum){
                if(data->next->content->sum[0]) free(data->next->content->sum[0]);
                if(data->next->content->sum[1]) free(data->next->content->sum[1]);
            }
            free(data->next->content);
        }
        free(data->next);
        data->next = NULL;
    }
    else{
        if(data->content){
            if(data->content->sum){
                if(data->content->sum[0]) free(data->content->sum[0]);
                if(data->content->sum[1]) free(data->content->sum[1]);
            }
            free(data->content);
        }
        free(&data);
        data = NULL;
    }
}

num* get_last(node* data){
    while(data->next)
        data = data->next;
    //printf("%s\n ", data->content->sum[0]);
    return data->content;
}

num* get_next(node** data){
    if((*data)->next)
        *data = (*data)->next;
    else return NULL;
    //printf("%s\n ", data->content->sum[0]);
    return (*data)->content;
}



int main(int argc, char** argv){
    MPI_Init(&argc, &argv); 

    int procrang, commsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;

    int pnum = atoi(argv[1]);
    int length = 1024/pnum;
    FILE* inf = fopen("in.txt", "rb");
    FILE* outf = fopen("out.txt", "w");

    char tnum1[1025];
    char tnum2[1026];
    char answer[1026];

    int t1;

    char _num1[length+1];
    char _num2[length+1];
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
            //printf("%d %d\n",i,  length/8);
            memcpy(_mnum1, (tnum1+(a-1)*8), 8); //copying a part of number to buffer
            memcpy(_mnum2, (tnum2+(a-1)*8), 8);
            _mnum1[8] = '\0';
            _mnum2[8] = '\0';
            memset(send, '\0', 18);
            memcpy(send, _mnum1, 9);
            memcpy((send+9), _mnum2, 9);

            //printf("%d: deal sent\n", procrang);
            MPI_Bcast(&interact, 1, MPI_INT, 0, MPI_COMM_WORLD ); //establishing connection and sending a part of number to first free process
            MPI_Recv(&interact, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            adress = status.MPI_SOURCE;
            //printf("%d: got %d \n", procrang, adress);
            //fflush(stdout);
            procmap[adress] = 1;
            //printf("%d: %d %d\n", procrang, procmap[adress], procmap[adress-1]);
            MPI_Scatter(procmap, 1, MPI_INT, &interact, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Send(send, 18, MPI_CHARACTER, adress, adress, MPI_COMM_WORLD);
            //printf("sent %s to %d\n", send, adress);
            //fflush(stdout);
            //break;
            sequence[a] = adress; //remembering the process order
            //printf("%d: %d | ", a, adress);
        }
        interact = -1;
        MPI_Bcast(&interact, 1, MPI_INT, 0, MPI_COMM_WORLD ); //telling all free processess there will be no job 

        add = 0;
        int ii = 0;
        MPI_Barrier(MPI_COMM_WORLD);
        for(int a = 1024/8; a > 0; a--){
            fflush(stdout);
            MPI_Send(&add, 1, MPI_INT, sequence[a], 100+sequence[a], MPI_COMM_WORLD);
            MPI_Recv(&add, 1, MPI_INT, sequence[a], 100+sequence[a], MPI_COMM_WORLD, &status);
            MPI_Recv(&rsum, 9, MPI_CHARACTER, sequence[a], 100+sequence[a], MPI_COMM_WORLD, &status);
            if(a>1) interact = 1;
            else interact = 0;
            MPI_Send(&interact, 1, MPI_INT, sequence[a], 100+sequence[a], MPI_COMM_WORLD);
            //printf("seq a-%d %d %s| ",a, sequence[a], rsum);
            memcpy((answer+(a-1)*8), rsum, 8);

            ii+=8;
            //printf("%s", rsum);
        }
        //MPI_Send(&add, 1, MPI_INT, sequence[a], 6, MPI_COMM_WORLD);
        answer[1024] = '\0';
        printf("\n%s\n", answer);
        fwrite(answer, 1024, 1, outf);
        //printf("%d", procrang);
        fflush(stdout);

    }
    else {
        //MPI_Bcast(&approve, 1, MPI_INT, 0, MPI_COMM_WORLD );
        //printf("%d: deal recieved\n", procrang);
        while(1){
            MPI_Bcast(&interact, 1, MPI_INT, 0, MPI_COMM_WORLD );
            if(interact == -1) break;
            //printf("%d: deal recieved\n", procrang);
            MPI_Send(&procrang, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            //printf("%d: deal sent\n", procrang);
            MPI_Scatter(procmap, 1, MPI_INT, &interact, 1, MPI_INT, 0, MPI_COMM_WORLD);
            //printf("%d: answ: %d\n", procrang, interact);
            fflush(stdout);
            if(interact == 0) continue;
            if(interact == -1) break;
            MPI_Recv(&send, 18, MPI_CHARACTER, 0, procrang, MPI_COMM_WORLD, &status);

            memcpy(_mnum1, send, 9);
            memcpy(_mnum2, (send+9), 9);
            //printf("summm: %d: %s %s\n", procrang, _mnum1, _mnum2);
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

            res = malloc(sizeof(num));
            res->sum[0] = malloc((9)*sizeof(char));
            res->sum[1] = malloc((9)*sizeof(char));
            res->add[0] = 0;
            res->add[1] = 0;

            memcpy((res->sum[0]), _sum, 9);
            memcpy((res->sum[1]), _sum1, 9);
            res->add[0] = add;
            res->add[1] = add1;

            //printf("%s %d\n%s %d\n", res->sum[0], res->add[0], res->sum[1], res->add[1]);
            add_end(data, res);
            
        }

        MPI_Barrier(MPI_COMM_WORLD);
        while(data){
            res = get_next(&data);
            if(!res) break;
            //printf("%d: res %s\n", procrang, res->sum[real_add]);
            fflush(stdout);
            MPI_Recv(&real_add, 1, MPI_INT, 0, 100+procrang, MPI_COMM_WORLD, &status);
            MPI_Send(&res->add[real_add], 1, MPI_INT, 0, 100+procrang, MPI_COMM_WORLD);
            MPI_Send(res->sum[real_add], 9, MPI_CHARACTER, 0, 100+procrang, MPI_COMM_WORLD);
            MPI_Recv(&interact, 1, MPI_INT, 0, 100+procrang, MPI_COMM_WORLD, &status);
            //delete_last(data);
            //if(!interact) break;
        }
        
        //printf("%d\n", procrang);

    }
    //printf("%d\n", procrang);
    MPI_Finalize();
}