/*
Первая строка файла - количество сортируемых чисел (< 2147483647), вторая строка - набор чисел (в пределах int), который необходимо отсортировать. Программе через аргумент сообщается, сколько использовать потоков. После считывания файла главным процессом (root), он отсылает остальным процессам участик числовой последовательности для сортировки. Каждый процесс сортирует числа на нужном количестве нитей. Затем происходит сортировка слиянием между процессами. В конце результат собирается в rootовом процессе и записывается в файл. В консоль программа выводит время работы алгоритма (от считывания файла и до вывода результата в файл).
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <mpi.h>
#include <string.h>

typedef struct mini_array{
    int* start;
    int len;
    int total_len;
    int num;
    int total_num;
} marr;

/*Heap data structure
1. Create an output array.
2. Create a min heap of size k and insert 1st element in all the arrays into the heap
3. Repeat following steps while priority queue is not empty.
a) Remove minimum element from heap (minimum is always at root) and store it in output array.
b) Insert next element from the array from which the element is extracted. If the array doesn’t have any more elements, then do nothing.
*/
// A pair of pairs, first element is going to 
// store value, second element index of array 
// and third element index in the array. 

typedef struct{
    int value;
    int outer_index;
    int inner_index;
} pair_t;

typedef struct {
    int priority;
    pair_t *data;
} node_t;
 
typedef struct {
    node_t *nodes;
    int len;
    int size;
} heap_t;
 
void push (heap_t *h, int priority, pair_t *data) {
    if (h->len + 1 >= h->size) {
        h->size = h->size ? h->size * 2 : 4;
        h->nodes = (node_t *) realloc(h->nodes, h->size * sizeof (node_t));
    }
    int i = h->len + 1;
    int j = i / 2;
    while (i > 1 && h->nodes[j].priority > priority) {
        h->nodes[i] = h->nodes[j];
        i = j;
        j = j / 2;
    }
    h->nodes[i].priority = priority;
    h->nodes[i].data = data;
    h->len++;
}
 
pair_t *pop (heap_t *h) {
    int i, j, k;
    if (!h->len) {
        return NULL;
    }
    pair_t *data = h->nodes[1].data;
 
    h->nodes[1] = h->nodes[h->len];
 
    h->len--;
 
    i = 1;
    while (i!=h->len+1) {
        k = h->len+1;
        j = 2 * i;
        if (j <= h->len && h->nodes[j].priority < h->nodes[k].priority) {
            k = j;
        }
        if (j + 1 <= h->len && h->nodes[j + 1].priority < h->nodes[k].priority) {
            k = j + 1;
        }
        h->nodes[i] = h->nodes[k];
        i = k;
    }
    return data;
}

int* mergeKArrays(int** arrays, int num, int* sizes, int len) { 
    int* output = calloc(len, sizeof(int)); 
    // Create a min heap with k heap nodes. Every prio
    // heap node has first element of an array 
    heap_t* pq = malloc(sizeof(heap_t));
    pq->size = 0;
    pq ->len = 0;
    pq ->nodes = malloc(sizeof(node_t));
    for(int i = 0; i < num; i++){
        pair_t* element = malloc(sizeof(pair_t));
        element->outer_index = i;
        element->inner_index = 0;
        element->value = arrays[i][0];
        push(pq, arrays[i][0], element); 
    }
    // Now one by one get the minimum element 
    // from min heap and replace it with next 
    // element of its array 
    int pos =0;
    while (pq->len > 0) { 
        pair_t* curr = pop(pq);   
        // i ==> Array Number 
        // j ==> Index in the array number         
        int i = curr->outer_index;   
        int j = curr->inner_index;  

        output[pos] = arrays[i][j];
        pos++;
        // The next element belongs to same array as  
        // current. 
        if (j +1 < sizes[i]){
            pair_t* element = malloc(sizeof(pair_t));
            element->outer_index = i;
            element->inner_index = j+1;
            element->value = arrays[i][j+1];
            push(pq, arrays[i][j+1], element); 
        }
    } 
  
    return output; 
}

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
    pointer = arg-> start;
    swap_to_end(pointer, arg->len);
}

void nchet(void* argm){
    marr* arg = (marr*) argm;
    int* pointer;
    pointer = arg-> start + 1;
    if(arg->num < arg->total_num-1)
        swap_to_end(pointer, arg->len);
    else{
        int fixpart = arg->total_len%2 ? 0: 1;
        swap_to_end(pointer, arg->len - fixpart);
    }
}

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
    }
    for(int count = 0; count < len; count ++){
        for(int t=0; t<nthreads; t++){ // creating threads
            if ((pthread_create(&threads[t], 0, chet, &arrs[t])) != 0) {
                printf("err creating thread %d", errno);
            }
            fflush(stdout);
        }
        for(int t = 0; t<nthreads; t++){
            pthread_join(threads[t], NULL);
        }
        for(int t=0; t<nthreads; t++){ // creating threads
            if ((pthread_create(&threads[t], 0, nchet, &arrs[t])) != 0) {
                printf("err creating thread %d", errno);
            }
            fflush(stdout);
        }
        for(int t = 0; t<nthreads; t++){
            pthread_join(threads[t], NULL);
        }
    }
    return array;
}

int main(int argc, char** argv){
    FILE* inf = fopen("sort_in.txt", "r");//reading numbers
    FILE* outf = fopen("sort_out.txt", "w");//reading numbers

    MPI_Init(&argc, &argv); 
    int procrang, commsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &procrang);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
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
    int counts[counternum];
    displs[0] = recvcounts[0] = 0;
    int** process_arrays = calloc(commsize, sizeof(int*));
    for(int t=1; t < commsize; t++){
            if(t < commsize-1) 
                counts[t-1] = recvcounts[t] = procmap[t*2] = l;
            else
                counts[t-1] = recvcounts[t] = procmap[t*2] = len - l*(t-1);
            displs[t] = procmap[t*2+1] = (t-1)*l;
    }
    if(procrang == 0){
        MPI_Scatter(procmap, 2, MPI_INT, &recv, 2, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gatherv(array, len, MPI_INT, recvbuf, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
        for(int t = 0; t<counternum; t++){
            process_arrays[t] = calloc(recvcounts[t+1], sizeof(int));
            memcpy(process_arrays[t], recvbuf + displs[t+1], recvcounts[t+1]*sizeof(int));
        }
        int* output = mergeKArrays(process_arrays, counternum, counts, len);
        print(output, len);
        printf("Time taken: %f s.", ((double) (clock()-t)/CLOCKS_PER_SEC));
        fflush(stdout);
        for(int a = 0; a < len; a++){
            fprintf(outf, "%d\n", array[a]);
        }
        fclose(outf);
    }
    else{
        MPI_Scatter(procmap, 2, MPI_INT, &recv, 2, MPI_INT, 0, MPI_COMM_WORLD);
        process_start = array + recv[1];
        process_len = recv[0];
        thread_manager(process_start, nthreads, process_len);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gatherv(process_start, process_len, MPI_INT, recvbuf, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}