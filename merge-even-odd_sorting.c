/*
Первая строка файла - количество сортируемых чисел (< 2147483647), вторая строка - набор чисел (в пределах int), который необходимо отсортировать. Программе через аргумент сообщается, сколько использовать потоков. После считывания файла главным процессом (root), он отсылает остальным процессам участик числовой последовательности для сортировки. Каждый процесс сортирует числа на нужном количестве нитей. Затем происходит сортировка слиянием между процессами. В конце результат собирается в rootовом процессе и записывается в файл. В консоль программа выводит время работы алгоритма (от считывания файла и до вывода результата в файл).
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <time.h>
#include <mpi.h>

typedef struct mini_array{
    int* arr;
    int* start;
    int len;
    int total_len;
    int num;
    int total_num;
} marr;

typedef struct macro_array{
    int* arr;
    int len;
} macarr;

void print(int* arr, int len){
    for(int a=0; a< len; a++){
        printf("%d ", arr[a]);
    }
    printf("\n");
}

int swap_to_end(int* arr, int len){
    int c;
    int *a, *b;
    int i;
    for (i = 0; i < len-1; i+=2) {
        a = (arr+i);
        b = (arr+i+1);
        if(*a > *b){
            c = *a;
            *a = *b;
            *b = c;
        }
    }
    return i;
}

void chet(void* argm){
    marr* arg = (marr*) argm;
    //printf("chet: ");
    //print(arg->arr, arg->len);
    int* pointer;
    pid_t tid = pthread_self();  
    //printf("%d %d\n", arg->num, arg->len);
    pointer = arg-> start;
    swap_to_end(pointer, arg->len);
    //printf("chet2: ");
    //print(arg->arr, arg->len);

}

void nchet(void* argm){
    marr* arg = (marr*) argm;
    //printf("nechet: ");
    //print(arg->arr, arg->len);
    int* pointer;
    pid_t tid = pthread_self();  
    //printf("%d %d\n", arg->num, arg->len);
    pointer = arg-> start + 1;
    if(arg->num < arg->total_num-1)
        swap_to_end(pointer, arg->len);
    else
        swap_to_end(pointer, arg->len-1);
    //printf("nechet2: ");
    //print(arg->arr, arg->len);

}

/*int* merge_sort(int *up, int *down, unsigned int left, unsigned int right){
    if (left == right){
        down[left] = up[left];
        return down;
    }

    unsigned int middle = (left + right) / 2;

    // разделяй и властвуй
    int *l_buff = merge_sort(up, down, left, middle);
    int *r_buff = merge_sort(up, down, middle + 1, right);

    // слияние двух отсортированных половин
    int *target = l_buff == up ? down : up;

    unsigned int l_cur = left, r_cur = middle + 1;
    for (unsigned int i = left; i <= right; i++){
        if (l_cur <= middle && r_cur <= right){
            if (l_buff[l_cur] < r_buff[r_cur]){
                target[i] = l_buff[l_cur];
                l_cur++;
            }
            else{
                target[i] = r_buff[r_cur];
                r_cur++;
            }
        }
        else if (l_cur <= middle){
            target[i] = l_buff[l_cur];
            l_cur++;
        }
        else{
            target[i] = r_buff[r_cur];
            r_cur++;
        }
    }
    return target;
    //print(up, right-left);
    //print(down, right-left);
    //printf("\n")
}

void* sort_manager(void* argm){
    marr* arg = (marr*) argm;
    //int* pointer;
    //printf("%d %d\n", arg->num, arg->len);
    //pointer = arg-> start;
    //print(arg->start, arg->len);
    int *down = calloc(sizeof(int), arg->len);
    down = merge_sort(arg->start, down, 0, arg->len-1);
    memcpy(arg->start, down, arg->len*sizeof(int)); //making counting for the case that 1 is added in the begining and the case it is not
    //print(arg->start, arg->len);
    fflush(stdout);
}
*/


int* thread_manager(int* array, int nthreads, int len){
    marr* arrs = calloc(nthreads, sizeof(marr));//allocating memory
    pthread_t threads[nthreads];
    int l = len/nthreads;
    for(int t=0; t < nthreads; t++){
        arrs[t].num = t;
        arrs[t].total_num = nthreads;
        arrs[t].total_len = len;

        if(t < nthreads-1){ 
            arrs[t].len = l;
        }
        else{
            arrs[t].len = len - (len/nthreads)*t;
        }
        arrs[t].start = array + t*l;
        arrs[t].arr = array;//calloc(len, sizeof(int));
        //printf("%d: %d + %d\n", arrs[t].num, *arrs[t].start, arrs[t].len);
    }

    //printf("loaded\n");
    //print(array, len);

    for(int count = 0; count < len; count ++){
        //print(array, len);
        for(int t=0; t<nthreads; t++){ // creating threads
            if ((pthread_create(&threads[t], 0, chet, &arrs[t])) != 0) {
                printf("err creating thread %d", errno);
            }
            //printf("\n");
            fflush(stdout);
        }
        for(int t = 0; t<nthreads; t++){
            pthread_join(threads[t], NULL);
        }
        //print(array, len);
        for(int t=0; t<nthreads; t++){ // creating threads
            if ((pthread_create(&threads[t], 0, nchet, &arrs[t])) != 0) {
                printf("err creating thread %d", errno);
            }
            //printf("\n");
            fflush(stdout);
        }
        for(int t = 0; t<nthreads; t++){
            pthread_join(threads[t], NULL);
        }
    //print(array, len);
    }
        print(array, len);

    return array;

}

int main(int argc, char** argv){
    FILE* inf = fopen("sort_in.txt", "r");//reading numbers
    FILE* outf = fopen("sort_out.txt", "w");//reading numbers

    MPI_Init(&argc, &argv); 

    int procrang, commsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Status status;


    int nthreads = atoi(argv[1]);
    int len;
    fscanf(inf, "%d", &len);
    int* array = calloc(len, sizeof(int));
    int element;
    for(int a = 0; a < len; a++){
        fscanf(inf, "%d", &element);
        array[a] = element;
    }
    clock_t t = clock();

    int counternum = commsize-1;
    int procmap[commsize*2];
    int recv[2];
    int l = len/counternum;

    int *process_start;
    int process_len;
    int* recvbuf = calloc(len, sizeof(int));
    int recvcounts[commsize];
    int displs[commsize];
    displs[0] = recvcounts[0] = 0;
    for(int t=1; t < commsize; t++){
            if(t < commsize-1) 
                recvcounts[t] = procmap[t*2] = l;
            else
                recvcounts[t] = procmap[t*2] = len - l*(t-1);
            displs[t] = procmap[t*2+1] = (t-1)*l;
            //printf("    %d %d %d %d\n", procmap[t*2], procmap[t*2+1], commsize, l);
    }
    //print(recvcounts, commsize);
    //print(displs, commsize);
    if(procrang == 0){
        MPI_Scatter(procmap, 2, MPI_INT, &recv, 2, MPI_INT, 0, MPI_COMM_WORLD); //giving the task to the process that answered first
        MPI_Gatherv(array, len, MPI_INT, recvbuf, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
        //thread_manager(array, nthreads, len);
        //printf("%d\n", procrang);
        print(recvbuf, len);
    }
    else{
        MPI_Scatter(procmap, 2, MPI_INT, &recv, 2, MPI_INT, 0, MPI_COMM_WORLD); //giving the task to the process that answered first
        process_start = array + recv[1];
        process_len = recv[0];
        //printf ("   %d %d %d %d\n", procrang, recv[1], *process_start, process_len);
        //printf("NOT 0 %d\n", procrang);
        thread_manager(process_start, nthreads, process_len);
        //print(process_start, process_len);
        MPI_Gatherv(process_start, process_len, MPI_INT, recvbuf, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
        fflush(stdout);
    }

    /*printf("Time taken: %f s.", ((double) (clock()-t)/CLOCKS_PER_SEC));
    fflush(stdout);
    for(int a = 0; a < len; a++){
        fprintf(outf, "%d\n", array[a]);
    }
    fclose(outf);*/
    MPI_Finalize();
}