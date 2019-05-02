/*
Первая строка файла - количество сортируемых чисел (< 2147483647), вторая строка - набор чисел (в пределах int), который необходимо отсортировать. Программе через аргумент сообщается, сколько использовать потоков. После считывания файла программа сортирует числа на нужном количестве нитей и записывает результат в файл. В консоль программа выводит время работы алгоритма (от считывания файла и до вывода результата в файл).
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>

typedef struct mini_array{
    int* arr;
    int* start;
    int len;
    int total_len;
    int num;
    int total_num;
    int left_index_c;
    int left_index_n;
    int* changed;
    int* neigh_changed;

    pthread_mutex_t* mutex; 
    pthread_mutex_t* omutex; 
    pthread_mutex_t** mutexes; 
    pthread_mutex_t** omutexes; 
    pthread_mutex_t* neigh_omutex; 
    pthread_mutex_t* neigh_mutex; 

} marr;

sem_t finish_sem;

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
    fflush(stdout);
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

int* threadFunc(void* argm){
    marr* arg = (marr*) argm;
    sem_wait(&finish_sem); 
    //printf("%d %d %d", arg->arr[0], arg->arr[1], arg->len);
    int unsorted = 1;
    int temp=0;
    int limit;
    if(arg->len < 2) return arg->arr;
    int* pointer;
	for(int count =0; count < arg->total_len*4; count++) {
        pointer = arg->start;
        arg->changed = 0;
        if(arg->len % 2 ==0){
            if( arg->mutex) pthread_mutex_lock(arg->mutex); //E
            if( arg->omutex) pthread_mutex_lock(arg->omutex);
            cmp_and_swap(pointer, (pointer+1), &arg->changed);
            //print(arg->arr, arg->total_len);
            //print(arg->start, arg->len);
            pointer +=2;
            pthread_mutex_unlock(arg->mutex);
            pointer += swap_to_end(pointer, arg->len - 1 , &arg->changed);
            //print(arg->arr, arg->total_len);
            //print(arg->start, arg->len);
            if(arg->num == 0) 
                pthread_mutex_unlock(arg->omutex);

            pointer = arg->start + 1;
            if( arg->omutex) pthread_mutex_lock(arg->omutex); //M
            pointer += swap_to_end(pointer, arg->len -1 , &arg->changed);
            if(arg->neigh_mutex)
                pthread_mutex_lock(arg->neigh_mutex);
            //print(arg->arr, arg->total_len);
            //print(arg->start, arg->len);
            if(arg->neigh_mutex){
                if(cmp_and_swap(pointer, (pointer+1), &arg->changed))
                    arg->neigh_changed = 1;
            }
            pointer+=2;
            if(arg->neigh_mutex)
                pthread_mutex_unlock(arg->neigh_mutex);
            pthread_mutex_unlock(arg->omutex); 
            if(arg->neigh_omutex)
               pthread_mutex_unlock(arg->neigh_omutex); 
            //fflush(stdout);
            //print(arg->arr, arg->total_len);
            //print(arg->start, arg->len);
            //printf("cycle end c %d\n", arg->changed);
        }
        //pthread_mutex_destroy(arg->neigh_mutex);
        //pthread_mutex_destroy(arg->neigh_omutex);
        //arg->neigh_mutex = NULL;
        //arg->neigh_omutex = NULL;
	}
    pthread_mutex_unlock(arg->mutex);
    pthread_mutex_unlock(arg->omutex);
    fflush(stdout);
    print(arg->arr, arg->total_len);
    sem_post(&finish_sem); 

    return arg->arr;
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
    pthread_mutex_t* mutexes = calloc(nthreads, sizeof(pthread_mutex_t));
    pthread_mutex_t* omutexes = calloc(nthreads, sizeof(pthread_mutex_t));
    
    sem_init(&finish_sem, 0, nthreads);

    for(int t=0; t < nthreads; t++){
        arrs[t].num = t;
        arrs[t].total_num = nthreads;
        arrs[t].total_len = len;
        arrs[t].changed = &changed[t];
        *arrs[t].changed = 1;
        arrs[t].mutex = &mutexes[t];
        arrs[t].omutex = &omutexes[t];
        pthread_mutex_init(arrs[t].mutex, NULL);
        pthread_mutex_init(arrs[t].omutex, NULL);

        if(t < nthreads-1){ 
            arrs[t].len = len/nthreads;
        }
        else{
            arrs[t].len = len - (len/nthreads)*t;
            arrs[t].neigh_omutex = NULL;
            arrs[t].neigh_mutex = NULL;
        }
        if(t > 0){
            arrs[t-1].neigh_omutex = arrs[t].omutex;
            arrs[t-1].neigh_mutex = arrs[t].mutex;
            arrs[t-1].neigh_changed = arrs[t].changed;
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

    /*oddEvenSorting(arrs[0].arr, arrs[0].len);
    memcpy(ret[0], arrs[0].arr, arrs[0].len*sizeof(int));*/

    threadFunc(&arrs[0]);

    sem_wait(&finish_sem); 
    fflush(stdout);
    //print(array, len);

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
