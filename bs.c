#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <string.h>

#define BUCKET_NUM 10
#define MAX_NUM 10000


void print_array(int arr[], int n)
{
   int i;
   for (i=0; i < n; i++)
       printf("%d ", arr[i]);
   printf("\n");
}

int cmp(const void *a, const void *b){
  return (*(int*)a < *(int*)b) ? -1 : (*(int*)a > *(int*)b);
}

int main(int argc, char** argv) {

  if(argc != 3) {
    printf("usage: bs <array_size> <bucket_num>\n");
    return 1;
  }

  int thread_num = omp_get_max_threads();
  long long array_size = atoll(argv[1])/thread_num*thread_num;
  int bucket_num = atoi(argv[2]);
  int bucket_range = MAX_NUM/bucket_num + 1;

  int* A = malloc(array_size * sizeof(int));

  #pragma omp parallel for
  for(int i = 0; i < thread_num; i++) {
    unsigned int seed = time(NULL) + i;
    for(long long j = 0; j < array_size/thread_num; j++) {
      A[j+i*array_size/thread_num] = rand_r(&seed)%MAX_NUM;
    }
  }

  // print_array(A, array_size);

  int** buckets = malloc(sizeof(int*) * bucket_num);
  int* buckets_ptrs = malloc(sizeof(long long) * bucket_num);
  for(int i = 0; i < bucket_num; i++) {
    buckets[i] = malloc(array_size * sizeof(int));
    buckets_ptrs[i] = 0;
  }

  #pragma omp parallel for
  for(long long i = 0; i < array_size; i++) {
    long long int j;
    for(j = 0; A[i] >= (j+1)*bucket_range; j++);
    long long idx;
    #pragma omp atomic capture
    idx = buckets_ptrs[j]++;
    buckets[j][idx] = A[i];
  }

  int* buckets_positions = malloc(sizeof(long long) * bucket_num);;
  buckets_positions[0] = 0;
  for(int i = 1; i < bucket_num; i++) {
    buckets_positions[i] = buckets_positions[i-1] + buckets_ptrs[i-1];
  }

  #pragma omp parallel for
  for(int i = 0; i < BUCKET_NUM; i++) {
    qsort(buckets[i], buckets_ptrs[i], sizeof(int), cmp);
    memcpy(A+buckets_positions[i], buckets[i], buckets_ptrs[i]*sizeof(int));
  }

  // print_array(A, array_size);

  for(long long i = 1; i < array_size; ++i)
    if(A[i-1] > A[i]){
      printf("\nFAILED!!! %d %d\n", A[i-1], A[i]);
      free(A);
      return -1;
    }

  free(A);

  return 0;
}
