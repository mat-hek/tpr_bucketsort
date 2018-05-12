#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <stdbool.h>

#define BUCKETS_NUM 1000


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


void merge_buckets(int *arr, Bucket **buckets, int *offsets, int threads_num, int rank){
    HeapNode heap_nodes[threads_num];
    HeapNode *heap[threads_num];
    Bucket *bucket;

    #pragma omp for schedule(dynamic)
    for(int bucket_num = 0; bucket_num < BUCKETS_NUM; ++bucket_num){
        int offset = offsets[bucket_num];
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
        }

        if(heap_size > 0){
            insertion_sort(heap, heap_size);
            while(heap[0]->idx >= 0){
                arr[offset++] = heap[0]->arr[heap[0]->idx--];
                shift_heap(heap, &heap_size);
            }
        }
    }
    return;
}


int cmp(const void *a, const void *b){
    return *(int*)b - *(int*)a;
}


static inline void sort_buckets(Bucket **buckets, int *offsets, int threads_num, int buckets_num){
    #pragma omp for reduction(+:offsets[:buckets_num]) schedule(dynamic)
    for(int i = 0; i < buckets_num * threads_num; ++i){
        int row = i / buckets_num;
        int col = i % buckets_num;
        Bucket *bucket = &buckets[row][col];
        int n_elems = bucket->len;
        for(int j = col + 1; j < buckets_num; ++j)
            offsets[j] += n_elems;
        qsort(bucket->arr, n_elems, sizeof(int), cmp);
    }
    return;
}


static inline void fill_buckets(int *arr, int arr_len, Bucket *buckets, int buckets_num){
    Bucket *bucket;
    int bucket_range = (INT_MAX / buckets_num) + 1;

    #pragma omp for schedule(static)
    for(int i = 0; i < arr_len; ++i){
        bucket = &buckets[arr[i] / bucket_range];
        bucket->arr[bucket->len++] = arr[i];
    }
    return;    
}


static inline Bucket *create_buckets(int bucket_size, int buckets_num){
    Bucket *buckets = malloc(sizeof(Bucket) * buckets_num);
    for(int i = 0; i < buckets_num; ++i)
        buckets[i] = (Bucket) {.len = 0, .arr = malloc(sizeof(int) * bucket_size)};
    return buckets;
}


static inline void free_buckets(Bucket *buckets, int buckets_num){
    for(int i = 0; i < buckets_num; ++i)
        free(buckets[i].arr);
    free(buckets);
    return;
}


void bucket_sort(int *arr, int arr_len, int buckets_num){
    Bucket *all_buckets[omp_get_max_threads()];
    int offsets[buckets_num];
    for(int i = 0; i < buckets_num; ++i)
        offsets[i] = 0;

    #pragma omp parallel shared(all_buckets, arr, arr_len, buckets_num)
    {
        int rank = omp_get_thread_num();
        int threads_num = omp_get_num_threads();

        Bucket *thread_buckets = create_buckets(arr_len, buckets_num);
        all_buckets[rank] = thread_buckets;

        fill_buckets(arr, arr_len, thread_buckets, buckets_num);
        sort_buckets(all_buckets, offsets, threads_num, buckets_num);
        merge_buckets(arr, all_buckets, offsets, threads_num, rank);

        free_buckets(thread_buckets, buckets_num);
    }
    return;
}


static inline void fill_array(int *arr, int arr_len){
    #pragma omp parallel
    {
        int seed = time(NULL) + omp_get_thread_num();

        #pragma omp for schedule(static)
        for(int i = 0; i < arr_len; ++i)
            arr[i] = rand_r(&seed);
    }
    return;
}


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Please insert the size of array");
        return -1;
    }

    int arr_len = atoll(argv[1]);
    int buckets_num = BUCKETS_NUM;
    int *arr = malloc(sizeof(int) * arr_len);

    fill_array(arr, arr_len);
//     print_array(arr, arr_len);
    bucket_sort(arr, arr_len, buckets_num);
//     print_array(arr, arr_len);

//     for(int i = 0; i < arr_len - 1; ++i)
//         if(arr[i] > arr[i + 1]){
//             printf("\nFAILED!!!\n");
//             free(arr);
//             return -1;
//         }

    free(arr);
    return 0;
}
