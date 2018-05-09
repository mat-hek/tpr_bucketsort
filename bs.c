#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <string.h>

#define BUCKET_NUM 10
#define MAX_NUM 10000
#define BUCKET_RANGE (MAX_NUM/BUCKET_NUM + 1)


void printArray(int arr[], int n)
{
   int i;
   for (i=0; i < n; i++)
       printf("%d ", arr[i]);
   printf("\n");
}

void insertionSort(int arr[], int n)
{
   int i, key, j;
   for (i = 1; i < n; i++)
   {
       key = arr[i];
       j = i-1;
       while (j >= 0 && arr[j] > key)
       {
           arr[j+1] = arr[j];
           j = j-1;
       }
       arr[j+1] = key;
   }
}


int main(int argc, char** argv) {

  if(argc != 2) {
    printf("usage: bs <array_size>");
    return 1;
  }

  int thread_num = omp_get_max_threads();
  int array_size = atoi(argv[1])/thread_num*thread_num;

  int* A = malloc(array_size * sizeof(int));

  #pragma omp parallel for
  for(int i = 0; i < thread_num; i++) {
    unsigned int seed = time(NULL) + i;
    for(int j = 0; j < array_size/thread_num; j++) {
      A[j+i*array_size/thread_num] = rand_r(&seed)%MAX_NUM;
    }
  }

  // printArray(A, array_size);

  int* buckets[BUCKET_NUM];
  int buckets_ptrs[BUCKET_NUM];
  for(int i = 0; i < BUCKET_NUM; i++) {
    buckets[i] = malloc(array_size * sizeof(int));
    buckets_ptrs[i] = 0;
  }

  #pragma omp parallel for shared(buckets_ptrs)
  for(int i = 0; i < array_size; i++) {
    long long int j;
    for(j = 0; A[i] >= (j+1)*BUCKET_RANGE; j++);
    buckets[j][buckets_ptrs[j]] = A[i];
    buckets_ptrs[j]++;
  }

  int buckets_positions[BUCKET_NUM];
  buckets_positions[0] = 0;
  for(int i = 1; i < BUCKET_NUM; i++) {
    buckets_positions[i] = buckets_positions[i-1] + buckets_ptrs[i-1];
  }

  #pragma omp parallel for
  for(int i = 0; i < BUCKET_NUM; i++) {
    insertionSort(buckets[i], buckets_ptrs[i]);
    memcpy(A+buckets_positions[i], buckets[i], buckets_ptrs[i]*sizeof(int));
  }

  // printArray(A, array_size);

  free(A);

  return 0;
}
