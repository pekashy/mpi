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
    int len;
    int total_len;
    int num;
    int total_num;
    int left_index_c;
    int left_index_n;

    pthread_mutex_t* cmutex; 
    pthread_mutex_t* nmutex; 
} marr;

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
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
}

int* threadFunc(void* argm){
    marr* arg = (marr*) argm;
    //printf("%d %d %d", arg->arr[0], arg->arr[1], arg->len);
    int unsorted = 1;
    int* array = arg->arr;
    int arrayLength = arg->len;
    int temp=0;
    int limit;
    int changed = 1;
	for (int i = 0; i < arg->total_len*2; i++) {
        changed = 0;
        print(arg->arr, arg->total_len);
        
        if(arg->nmutex) pthread_mutex_lock(arg->nmutex);
        if(arg->cmutex) pthread_mutex_unlock(arg->cmutex);
        if(arg->num == arg->total_num)
            limit = arg->left_index_c+arrayLength-1;
        else 
            limit = arg->left_index_c+arrayLength;
        for(int j=arg->left_index_c; j < limit; j+=2){
            if (array[j] > array[j + 1]) {
                temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
                changed = 1;
			}
            print(arg->arr, arg->total_len);
        }
        printf("changed c: %d\n", changed);

        if(arg->cmutex) pthread_mutex_lock(arg->cmutex);

        if(arg->num == arg->total_num)
            limit = arg->left_index_n+arrayLength-1;
        else 
            limit = arg->left_index_n+arrayLength;

        for(int j=arg->left_index_n; j < limit; j+=2){
            if (array[j] > array[j + 1] && ((j + 1) < arg->left_index_n+arrayLength-1) && arg->num != arg->total_num) {
                temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
                changed = 1;
			}
            print(arg->arr, arg->total_len);
        }
        //print(arg->arr, arg->total_len);
        printf("changed n: %d\n", changed);

        if(arg->nmutex) pthread_mutex_unlock(arg->nmutex);
        //if(changed == 0) break;

	    // (i % 2) ? 0 : 1 возвращает 1, если i четное, 0, если i не четное
		/*for (int j = (i % 2) ? 0 : 1; j < arrayLength - 1; j += 2) {
			if (array[j] > array[j + 1]) {
                temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
			}
		}*/
	}
    //if(arg->cmutex) pthread_mutex_unlock(arg->cmutex);
    printf("thread finished\n");
    fflush(stdout);
    //if(arg->nmutex) pthread_mutex_unlock(arg->nmutex);

    return array;
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

    marr* arrs = calloc(nthreads, sizeof(marr));//allocating memory
    int* ret[nthreads];
    pthread_t threads[nthreads];
    pthread_mutex_t* mutexes = calloc(nthreads-1, sizeof(pthread_mutex_t));
    for(int t=0; t < nthreads; t++){
        arrs[t].num = t;
        arrs[t].total_num = nthreads;
        arrs[t].total_len = len;
        if(t < nthreads-1){ 
            arrs[t].len = len/nthreads+1;
            arrs[t].cmutex = NULL;
            arrs[t].nmutex = &mutexes[t];
            //pthread_mutex_init(arrs[t].nmutex, NULL);
        }
        else{
            arrs[t].len = len - (len/nthreads+1)*t;
            arrs[t].nmutex = NULL;
        }
        //printf("%d: %d\n", t, arrs[t].len);
        if(t > 0){
            arrs[t].cmutex = &mutexes[t-1];
            pthread_mutex_init(arrs[t].cmutex, NULL);
            pthread_mutex_lock(arrs[t].cmutex);
        }
        

        arrs[t].arr = array;//calloc(len, sizeof(int));
        arrs[t].left_index_c = t*(len/nthreads+1);
        arrs[t].left_index_n = arrs[t].left_index_c+1;
        //printf("%d: %d %d\n", t, arrs[t].left_index_c, arrs[t].left_index_c+arrs[t].len-1);
        /*memcpy(arrs[t].arr, array, len*sizeof(int));  
        ret[t] = calloc(len/nthreads+1, sizeof(int));  */
    }

    for(int t=1; t<nthreads; t++){ // creating threads
        if ((pthread_create(&threads[t], 0, threadFunc, &arrs[t])) != 0) {
            printf("err creating thread %d", errno);
        }
    }

    /*oddEvenSorting(arrs[0].arr, arrs[0].len);
    memcpy(ret[0], arrs[0].arr, arrs[0].len*sizeof(int));*/

    threadFunc(&arrs[0]);
    int* r = calloc(len, sizeof(int));

    for(int t=1; t<nthreads; t++){ //reasing results to ret
        if(!threads[t]) continue;
        pthread_join(threads[t], &r);
    }

    //print(r, len);

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