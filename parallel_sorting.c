
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

typedef struct mini_array{
    int* arr;
    int* start;
    int len;
    int total_len;
    int num;
    int total_num;
    int left_index_c;
    int left_index_n;

} marr;

sem_t mutex1;
sem_t barrier1;
sem_t mutex2;
sem_t barrier2;
sem_t mutex3;
sem_t barrier3;
sem_t mutex4;
sem_t barrier4;
sem_t mutex5;
sem_t barrier5;

int count1 = 0;
int count2 = 0;
int count3 = 0;
int count4 = 0;
int count5 = 0;

void oddEvenSorting(int* array, int arrayLength) {
    int temp;
	for (int i = 0; i < arrayLength; i++) {
	    // (i % 2) ? 0 : 1 возвращает 1, если i четное, 0, если i не четное
		for (int j = (i % 2) ? 0 : 1; j < arrayLength - 1; j += 2) {
			if (array[j] > array[j + 1]) {
                temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
			}
		}
	}
}

void print(int* arr, int len){
    for(int a=0; a< len; a++){
        printf("%d ", arr[a]);
    }
    printf("\n");
}

int cmp_and_swap(int* a, int* b, int* changed){
    int c;
    if(*a > *b){
        c = *a;
        *a = *b;
        *b = c;
        *changed = 1;
        return 1;
    }
    return 0;
}


int swap_to_end(int* arr, int len, int* changed){
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
            *changed = 1;
        }
    }
    return i;
}

void bar(marr* arg, int id){
    if(id ==1){
        count1 = count1 + 1;
        sem_post(&mutex1);
        if(count1 == arg->total_num) sem_post(&barrier1);
        sem_wait(&barrier1);
        sem_post(&barrier1); //# once we are unblocked, it's our duty to unblock the next thread
    }
    if(id == 2){
        count2 = count2 + 1;
        sem_post(&mutex2);
        if(count2 == arg->total_num) sem_post(&barrier2);
        sem_wait(&barrier2);
        sem_post(&barrier2); //# once we are unblocked, it's our duty to unblock the next thread
    }
    if(id == 3){
            count3 = count3 + 1;
            sem_post(&mutex3);
            if(count3 == arg->total_num) sem_post(&barrier3);
            sem_wait(&barrier3);
            sem_post(&barrier3); //# once we are unblocked, it's our duty to unblock the next thread
    }
    if(id == 4){
            count4 = count4 + 1;
            sem_post(&mutex4);
            if(count4 == arg->total_num) sem_post(&barrier4);
            sem_wait(&barrier4);
            sem_post(&barrier4); //# once we are unblocked, it's our duty to unblock the next thread
    }
    if(id == 5){
            count5 = count5 + 1;
            sem_post(&mutex5);
            if(count5 == arg->total_num) sem_post(&barrier5);
            sem_wait(&barrier5);
            sem_post(&barrier5); //# once we are unblocked, it's our duty to unblock the next thread
    }

}
/*
sem1, sem2 =0


rank == 0
    sem1 += p-1
    sem2 -= p-1
    
rank != 0
    sem2 += 1

rank != 0
    sem1 -= 1


*/
void threadFunc(void* argm){
    marr* arg = (marr*) argm;
    int unsorted = 1;
    int temp=0;
    int limit;
    if(arg->len < 2) return arg->arr;
    int* pointer;
    fflush(stdout);
    int changed;
    pid_t tid = pthread_self();  
    printf("%d %d\n", arg->num, arg->len);
    for(int c = 0; c < arg->total_len; c++) {

        if(arg->len%2 == 0){
            pointer = arg-> start;
            swap_to_end(pointer, arg->len, &changed);
        }
        else{
            pointer = arg-> start;
            if(arg->num < arg->total_num) swap_to_end(pointer, arg->len, &changed);
            else swap_to_end(pointer, arg->len-1, &changed);
        }


        sem_init(&mutex3, 0, 1);
        sem_init(&barrier3, 0, 0);
        count3 = 0;
            bar(arg, 1);
        fflush(stdout);
        //printf("barrier %d: %d 1\n", tid, c);
        //printf("%d %d %d %d 1\n", count1, count2, count3, count4);
        fflush(stdout);

        sem_init(&mutex3, 0, 1);
        sem_init(&barrier3, 0, 0);
        count3 = 0;
            bar(arg, 2);
        fflush(stdout);
        //printf("barrier %d: %d 2\n", tid, c);
        //printf("%d %d %d %d 2\n", count1, count2, count3, count4);

        fflush(stdout);

        if(arg->len%2 == 0){
            pointer = arg-> start + 1;
            if(arg->num< arg->total_num) swap_to_end(pointer, arg->len, &changed);
            else swap_to_end(pointer, arg->len-1, &changed);
        }
        else{
            pointer = arg-> start + !arg->num%2;
            fflush(stdout);
            printf("%d\n", !arg->num%2);
                    fflush(stdout);
            if(arg->num< arg->total_num) swap_to_end(pointer, arg->len - !arg->num%2, &changed);
            else swap_to_end(pointer, arg->len-1, &changed);
        }



        sem_init(&mutex1, 0, 1);
        sem_init(&barrier1, 0, 0);
        count1 = 0;
        sem_init(&mutex4, 0, 1);
        sem_init(&barrier4, 0, 0);
        count4 = 0;
            bar(arg, 3);
        fflush(stdout);
        fflush(stdout);
       // printf("barrier %d: %d 3\n", tid, c);
        //printf("%d %d %d %d 3\n", count1, count2, count3, count4);
        fflush(stdout);

        sem_init(&mutex2, 0, 1);
        sem_init(&barrier2, 0, 0);
        count2 = 0;
            bar(arg, 4);
        fflush(stdout);
        fflush(stdout);
        //printf("barrier %d: %d 4\n", tid, c);
        //printf("%d %d %d %d 4\n", count1, count2, count3, count4);
        //print(arg->arr, arg->total_len);
        fflush(stdout);
    }
}


int main(int argc, char** argv){
    FILE* inf = fopen("sort_in.txt", "r");//reading numbers
    int nthreads = atoi(argv[1]);
    int len;
    fscanf(inf, "%d", &len);
    int* array = calloc(len, sizeof(int));
    int element;
    for(int a = 0; a < len; a++){
        fscanf(inf, "%d", &element);
        array[a] = element;
    }
    int* changed = calloc(nthreads, sizeof(int));
    marr* arrs = calloc(nthreads, sizeof(marr));//allocating memory
    int* ret[nthreads];
    pthread_t threads[nthreads];

    sem_init(&mutex1, 0, 1);
    sem_init(&barrier1, 0, 0);
    sem_init(&mutex2, 0, 1);
    sem_init(&barrier2, 0, 0);
    sem_init(&mutex3, 0, 1);
    sem_init(&barrier3, 0, 0);
    sem_init(&mutex4, 0, 1);
    sem_init(&barrier4, 0, 0);
    sem_init(&mutex5, 0, 1);
    sem_init(&barrier5, 0, 0);

    for(int t=0; t < nthreads; t++){
        arrs[t].num = t;
        arrs[t].total_num = nthreads;
        arrs[t].total_len = len;

        if(t < nthreads-1){ 
            arrs[t].len = len/nthreads;
        }
        else{
            arrs[t].len = len - (len/nthreads)*t;
        }
        arrs[t].arr = array;//calloc(len, sizeof(int));
        arrs[t].start = array + t*arrs[t].len;//calloc(len, sizeof(int));
        arrs[t].left_index_c = t*(len/nthreads+1);
        arrs[t].left_index_n = arrs[t].left_index_c+1;
        
    }

    printf("loaded\n");
    for(int t=1; t<nthreads; t++){ // creating threads
        if ((pthread_create(&threads[t], 0, threadFunc, &arrs[t])) != 0) {
            printf("err creating thread %d", errno);
        }
    }


    threadFunc(arrs);

    fflush(stdout);
    print(array, len);



    /*int piece;
    for(int a=0; a < nthreads; a++){ // printing results
        if(a < nthreads-1) 
            piece = len/nthreads+1;
        else 
            piece = len - (len/nthreads+1)*a;
        for(int b=0; b < piece; b++){
            printf("a: %d\n", ret[a][b]);
        }
    }*/
}