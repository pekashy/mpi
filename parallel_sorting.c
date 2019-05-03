/*
Первая строка файла - количество сортируемых чисел (< 2147483647), вторая строка - набор чисел (в пределах int), который необходимо отсортировать. Программе через аргумент сообщается, сколько использовать потоков. После считывания файла программа сортирует числа на нужном количестве нитей и записывает результат в файл. В консоль программа выводит время работы алгоритма (от считывания файла и до вывода результата в файл).
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <time.h>

typedef struct mini_array{
    int* arr;
    int* start;
    int len;
    int total_len;
    int num;
    int total_num;
} marr;


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
    int* pointer;
    pid_t tid = pthread_self();  
    //printf("%d %d\n", arg->num, arg->len);
    pointer = arg-> start;
    swap_to_end(pointer, arg->len);
}

void nchet(void* argm){
    marr* arg = (marr*) argm;
    int* pointer;
    pid_t tid = pthread_self();  
    //printf("%d %d\n", arg->num, arg->len);
    pointer = arg-> start + 1;
    if(arg->num < arg->total_num)
        swap_to_end(pointer, arg->len);
    else
        swap_to_end(pointer, arg->len-1);

}

int main(int argc, char** argv){
    FILE* inf = fopen("sort_in.txt", "r");//reading numbers
    FILE* outf = fopen("sort_out.txt", "w");//reading numbers

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

    int* changed = calloc(nthreads, sizeof(int));
    marr* arrs = calloc(nthreads, sizeof(marr));//allocating memory
    pthread_t threads[nthreads];

    for(int t=0; t < nthreads; t++){
        arrs[t].num = t;
        arrs[t].total_num = nthreads;
        arrs[t].total_len = len;

        if(t < nthreads-1){ 
            arrs[t].len = len/nthreads;
            arrs[t].start = array + (t)*arrs[t].len;
        }
        else{
            arrs[t].len = len - (len/nthreads)*t;
            arrs[t].start = array + (t)*arrs[t-1].len;
        }
        arrs[t].arr = array;//calloc(len, sizeof(int));
        //printf("%d: %d + %d\n", arrs[t].num, *arrs[t].start, arrs[t].len);
    }

    printf("loaded\n");
    //print(array, len);

    for(int count = 0; count < len; count ++){
        for(int t=0; t<nthreads; t++){ // creating threads
            if ((pthread_create(&threads[t], 0, chet, &arrs[t])) != 0) {
                printf("err creating thread %d", errno);
            }
        }

        for(int t = 0; t<nthreads; t++){
            pthread_join(threads[t], NULL);
        }
        //print(array, len);
        for(int t=0; t<nthreads; t++){ // creating threads
            if ((pthread_create(&threads[t], 0, nchet, &arrs[t])) != 0) {
                printf("err creating thread %d", errno);
            }
        }

        for(int t = 0; t<nthreads; t++){
            pthread_join(threads[t], NULL);
        }
        //print(array, len);

    }
    printf("Time taken: %f s.", ((double) (clock()-t)/CLOCKS_PER_SEC));
    fflush(stdout);
    //print(array, len);

    for(int a = 0; a < len; a++){
        fprintf(outf, "%d\n", array[a]);
    }
    fclose(outf);
}