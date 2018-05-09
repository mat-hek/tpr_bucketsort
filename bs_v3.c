#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <stdbool.h>

#define BUCKETS_NUM 10


typedef struct{
    int len;
    int *arr;
} Bucket;


typedef struct{
    int idx;
    int *arr;
} HeapNode;


void print_array(int *arr, int arr_len){
    printf("\n[");
    for(int i = 0; i < arr_len - 1; ++i)
        printf("%d, ", arr[i]);
    printf("%d]\n", arr[arr_len - 1]);
    return;
}


void insertion_sort(HeapNode **heap, int heap_size){
    for(int i = 1; i < heap_size; ++i){
        HeapNode *node = heap[i];
        int j = i - 1;
        for(; j >= 0; --j){
            if(heap[j]->arr[heap[j]->idx] <= node->arr[node->idx]){
                heap[j + 1] = node;
                break;
            }
            heap[j + 1] = heap[j];
            heap[j] = node;
        }
    }
}


void shift_heap(HeapNode **heap, int *heap_size){
    HeapNode *node;
    if(heap[0]->idx < 0 && *heap_size > 1){
        node = heap[0];
        heap[0] = heap[*heap_size - 1];
        heap[*heap_size - 1] = node;
        *heap_size -= 1;
    }

    node = heap[0];
    for(int i = 1; i < *heap_size; ++i){
        if(heap[i]->arr[heap[i]->idx] >= node->arr[node->idx]){
            heap[i - 1] = node;
            break;
        }
        heap[i - 1] = heap[i];
        heap[i] = node;
    }

    return;
}


void merge_buckets(int *arr, int arr_len, Bucket **buckets, int threads_num, int rank){
    HeapNode *heap_nodes = malloc(sizeof(HeapNode) * threads_num);
    HeapNode **heap = malloc(sizeof(HeapNode *) * threads_num);
    Bucket *bucket;

    #pragma omp for schedule(dynamic)
    for(int bucket_num = 0; bucket_num < BUCKETS_NUM; ++bucket_num){
        int offset = 0;
        int heap_size = 0;

        for(int i = 0; i < threads_num; ++i){
            bucket = &buckets[i][bucket_num];

            int idx = bucket->len - 1;
            if(idx >= 0){
                heap_nodes[heap_size].arr = bucket->arr;
                heap_nodes[heap_size].idx = idx;
                heap[heap_size] = &heap_nodes[heap_size];
                ++heap_size;
            }
            for(int j = 0; j < bucket_num; ++j)
                offset += buckets[i][j].len;
        }

        if(heap_size > 0){
            insertion_sort(heap, heap_size);
            while(heap[0]->idx >= 0){
                arr[offset++] = heap[0]->arr[heap[0]->idx--];
                shift_heap(heap, &heap_size);
            }
        }
    }

    free(heap_nodes);    
    free(heap);
    return;
}


int cmp(const void *a, const void *b){    
    return *(int*)b - *(int*)a;
}


void sort_buckets(Bucket **buckets, int threads_num){
    #pragma omp for schedule(dynamic)
    for(int i = 0; i < BUCKETS_NUM * threads_num; ++i){
        int row = i / BUCKETS_NUM;
        int col = i % BUCKETS_NUM;
        Bucket *bucket = &buckets[row][col];
        qsort(bucket->arr, bucket->len, sizeof(int), cmp);
    }
    return;
}


void fill_buckets(int *arr, int arr_len, Bucket *buckets){
    Bucket *bucket;
    int bucket_range = (INT_MAX / BUCKETS_NUM) + 1;

    #pragma omp for schedule(static)
    for(int i = 0; i < arr_len; ++i){
        bucket = &buckets[arr[i] / bucket_range];
        bucket->arr[bucket->len] = arr[i];
        bucket->len += 1;
    }
    return;    
}


void bucket_sort(int *arr, int arr_len){
    Bucket **buckets = malloc(sizeof(Bucket *) * omp_get_max_threads());

    #pragma omp parallel shared(buckets, arr, arr_len)
    {
        int rank = omp_get_thread_num();
        int threads_num = omp_get_num_threads();
        int bucket_size = (arr_len / threads_num) + threads_num;

        Bucket *thread_buckets = malloc(sizeof(Bucket) * BUCKETS_NUM);
        for(int i = 0; i < BUCKETS_NUM; ++i){
            thread_buckets[i].len = 0;
            thread_buckets[i].arr = malloc(sizeof(int) * bucket_size);
        }
        buckets[rank] = thread_buckets;

        fill_buckets(arr, arr_len, thread_buckets);
        sort_buckets(buckets, threads_num);
        merge_buckets(arr, arr_len, buckets, threads_num, rank);

        for(int i = 0; i < BUCKETS_NUM; ++i)
            free(buckets[rank][i].arr);
        free(buckets[rank]);
    }

    free(buckets);
    return;
}


void fill_array(int *arr, int arr_len){
    #pragma omp parallel
    {
        int seed = time(NULL) + omp_get_thread_num();

        #pragma omp for schedule(static)
        for(int i = 0; i < arr_len; ++i){
            arr[i] = rand_r(&seed);
        }
    }
    return;
}


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Please insert the size of array");
        return -1;
    }

    int arr_len = atoi(argv[1]);
    int *arr = malloc(sizeof(int) * arr_len);

    fill_array(arr, arr_len);
//     print_array(arr, arr_len);
    bucket_sort(arr, arr_len);
//     print_array(arr, arr_len);

    for(int i = 0; i < arr_len - 1; ++i)
        if(arr[i] > arr[i + 1]){
            printf("\nFAILED!!!\n");
            break;
        }

    free(arr);
    return 0;
}
