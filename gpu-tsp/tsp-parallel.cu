#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

/* Original permuation code due to D. Jimenez, UT Austin
 * http://faculty.cse.tamu.edu/djimenez/ut/utsa/cs3343/
 */

/* Reference an element in the TSP distance array. */
#define TSP_ELT(tsp, n, i, j) *(tsp + (i * n) + j)

/* Create an instance of a symmetric TSP. */
int *
create_tsp(int n, int random_seed)
{
  int *tsp = (int *) malloc(n * n * sizeof(int));

  srandom(random_seed);
  for (int i = 0;  i < n;  i++) {
	for (int j = 0;  j <= i;  j++) {
	  int val = (int)(random() / (RAND_MAX / 100));
	  TSP_ELT(tsp, n, i, j) = val;
	  TSP_ELT(tsp, n, j, i) = val;
	}
  }
  return tsp;
}

/* Print a TSP distance matrix. */
void
print_tsp(int *tsp, int n, int random_seed)
{
  printf("TSP (%d cities - seed %d)\n    ", n, random_seed);
  for (int j = 0;  j < n;  j++) {
	printf("%3d|", j);
  }
  printf("\n");
  for (int i = 0;  i < n;  i++) {
	printf("%2d|", i);
	for (int j = 0;  j < n;  j++) {
	  printf("%4d", TSP_ELT(tsp, n, i, j));
	}
	printf("\n");
  }
  printf("\n");
}

void
usage(char *prog_name)
{
  fprintf(stderr, "usage: %s [flags]\n", prog_name);
  fprintf(stderr, "   -h\n");
  fprintf(stderr, "   -t <number of threads>\n");
  fprintf(stderr, "   -c <number of cities>\n");
  fprintf(stderr, "   -s <random seed>\n");
  exit(1);
}

__device__ unsigned long
factorial(int n)
{
    if (n < 1) {
        return 0;
    }

    unsigned long rtn = 1;
    for (unsigned i = 1;  i <= n;  i++) {
        rtn *= i;
    }
    return rtn;
}


__device__ int 
calc_cost(int *perm, int *matrix, int n)
{
    int total = 0;
    for (int i = 0;  i < n;  i++) {
        int j = (i + 1) % n;
        int from = perm[i];
        int to = perm[j];
        int val = TSP_ELT(matrix, n, from, to);
        total += val;
    }

    return total;
}

void 
create_tsp(int *matrix, int n, int random_seed)
{
    srandom(random_seed);
    for (int i = 0;  i < n;  i++) {
        for (int j = 0;  j <= i;  j++) {
            int val = (int)(random() / (RAND_MAX / 100));
            TSP_ELT(matrix, n, i, j) = val;
            TSP_ELT(matrix, n, j, i) = val;
        }
    }
}

/**** List ADT ****************/

typedef struct {
  int *values;					/* Values stored in list */
  int max_size;					/* Maximum size allocated */
  int cur_size;					/* Size currently in use */
} list_t;

/* Dump list, including sizes */
__device__ void
list_dump(list_t *list)
{
  printf("%2d/%2d", list->cur_size, list->max_size);
  for (int i = 0;  i < list->cur_size;  i++) {
	printf(" %d", list->values[i]);
  }
  printf("\n");
}

/* Allocate list that can store up to 'max_size' elements */
__device__ list_t *
list_alloc(int max_size)
{
  list_t *list = (list_t *)malloc(sizeof(list_t));
  list->values = (int *)malloc(max_size * sizeof(int));
  list->max_size = max_size;
  list->cur_size = 0;
  return list;
}

/* Free a list; call this to avoid leaking memory! */
__device__ void
list_free(list_t *list)
{
  free(list->values);
  free(list);
}

/* Add a value to the end of the list */
__device__ void
list_add(list_t *list, int value)
{
  if (list->cur_size >= list->max_size) {
	printf("List full");
	list_dump(list);
  }
  list->values[list->cur_size++] = value;
}

/* Return the current size of the list */
__device__ int
list_size(list_t *list)
{
  return list->cur_size;
}

/* Validate index */
__device__ void
_list_check_index(list_t *list, unsigned long index)
{
  if (index > list->cur_size - 1) {
	printf("Invalid index %d\n", index);
	list_dump(list);
  }
}

/* Get the value at given index */
__device__ int
list_get(list_t *list, unsigned long index)
{
  _list_check_index(list, index);
  return list->values[index];
}

/* Remove the value at the given index */
__device__ void
list_remove_at(list_t *list, int index)
{
  _list_check_index(list, index);
  for (int i = index; i < list->cur_size - 1;  i++) {
	list->values[i] = list->values[i + 1];
  }
  list->cur_size--;
}

/* Retrieve a copy of the values as a simple array of integers. The returned
   array is allocated dynamically; the caller must free the space when no
   longer needed.
 */
__device__ int *
list_as_array(list_t *list)
{
  int *rtn = (int *)malloc(list->max_size * sizeof(int));
  for (int i = 0;  i < list->max_size;  i++) {
	rtn[i] = list_get(list, i);
  }
  return rtn;
}

/**** Permutation ****************/

/* Permutation algorithms based on code found at:
   http://www.mathblog.dk/project-euler-24-millionth-lexicographic-permutation/
   which references:
   http://www.cut-the-knot.org/do_you_know/AllPerm.shtml
*/

/* Return the kth lexographically ordered permuation of an array of k integers
   in the range [0 .. size - 1]. The integers are allocated dynamically and
   should be free'd by the caller when no longer needed.
*/
__device__ int *
kth_perm(int k, int size)
{
  long remain = k - 1;

  list_t *numbers = list_alloc(size);
  for (int i = 0;  i < size;  i++) {
	list_add(numbers, i);
  }

  list_t *perm = list_alloc(size);

  for (int i = 1;  i < size;  i++) {
	unsigned long f = factorial(size - i);
	unsigned long j = remain / f;
	remain = remain % f;

	list_add(perm, list_get(numbers, j));
	list_remove_at(numbers, j);

	if (remain == 0) {
	  break;
	}
  }

  /* Append remaining digits */
  for (int i = 0;  i < list_size(numbers);  i++) {
	list_add(perm, list_get(numbers, i));
  }

  int *rtn = list_as_array(perm);
  list_free(perm);

  return rtn;
}

/* Swap v[i] and v[j] */
__device__ void
swap(int *v, int i, int j)
{
  int t = v[i];
  v[i] = v[j];
  v[j] = t;
}

/* Print a permutation array */
__device__ void
print_perm(int *perm, int size)
{
  for (int k = 0; k < size; k++) {
	printf("%4d", perm[k]);
  }
  printf("\n");
}

/* Given an array of size elements at perm, update the array in place to
   contain the lexographically next permutation. It is originally due to
   Dijkstra. The present version is discussed at:
   http://www.cut-the-knot.org/do_you_know/AllPerm.shtml
 */
__device__ void
next_perm(int *perm, int size)
{
  int i = size - 1;
  while (perm[i - 1] >= perm[i]) {
	i = i - 1;
  }

  int j = size;
  while (perm[j - 1] <= perm[i - 1]) {
	j = j - 1;
  }

  swap(perm, i - 1, j - 1);

  i++;
  j = size;
  while (i < j) {
	swap(perm, i - 1, j - 1);
	i++;
	j--;
  }
}


__global__ void
perm_kernel(int *glob_cost_matrix, int *min_matrix, int num_cities, int num_threads)
{
    int block_id = 
        blockIdx.x +
        blockIdx.y * blockDim.x +
        blockIdx.z * blockDim.x * blockDim.y;
    int block_offset =
        block_id *
        blockDim.x * blockDim.y * blockDim.z;
    int thread_offset =
        threadIdx.x +
        threadIdx.y * blockDim.x +
        threadIdx.z * blockDim.x * blockDim.y;
    int tid = block_offset + thread_offset;
    __shared__ int cost_matrix[144];
    int init_iters = (num_cities * num_cities) / num_threads;
    if(tid == 0)
        init_iters += (num_cities * num_cities) % num_threads;
    for(int i = 0; i < num_cities * num_cities; i++)
        cost_matrix[i] = glob_cost_matrix[i];
    __syncthreads();
    unsigned long num_iters = factorial(num_cities) / num_threads;
    int *perm = kth_perm((num_iters * tid) + 1, num_cities);

    int min_cost = INT_MAX;
    int cost;
    for(unsigned long i = 0; i < num_iters; i++)
    {
        cost = calc_cost(perm, cost_matrix, num_cities);
        if(cost < min_cost)
        {
            min_cost = cost;
        }
        next_perm(perm, num_cities);
    }

    min_matrix[tid] = min_cost;
}

double
now(void)
{
    struct timespec current_time;
    double ONE_BILLION = (double)1000000000.0;

    clock_gettime(CLOCK_REALTIME, &current_time);
    return current_time.tv_sec + (current_time.tv_nsec / ONE_BILLION);
}

int
main(int argc, char **argv)
{
  int num_cities = 3;
  int random_seed = 42;
  int num_threads = 5;

  int ch;
  while ((ch = getopt(argc, argv, "t:c:hs:")) != -1) {
	switch (ch) {
	case 'c':
	  num_cities = atoi(optarg);
	  break;
	case 's':
	  random_seed = atoi(optarg);
	  break;
	case 't':
	  num_threads = atoi(optarg);
	  break;
	case 'h':
	default:
	  usage(argv[0]);
	}
  }
  double start = now();

  int cost_matrix_size = sizeof(int) * num_cities * num_cities;
  int min_matrix_size = sizeof(int) * num_threads;

  //Initialize matrices
  int *min_matrix_h = (int *) malloc(min_matrix_size);
  int *cost_matrix_h = (int *) malloc(cost_matrix_size);
  int *min_matrix_d, *cost_matrix_d;
  cudaMalloc(&min_matrix_d, min_matrix_size);
  cudaMalloc(&cost_matrix_d, cost_matrix_size);

  //create and copy cost matrix to device
  create_tsp(cost_matrix_h, num_cities, random_seed);
//  print_tsp(cost_matrix_h, num_cities, random_seed);
  cudaMemcpy(cost_matrix_d, cost_matrix_h, cost_matrix_size, cudaMemcpyHostToDevice);

  //launch kernel
  int threads_per_block = num_threads;
  for(int i = 1; i < 1025; i *= 2)
  {
      if(i > num_threads)
          break;
      if(i < num_threads && (num_threads % i == 0))
      {
          threads_per_block = i;
      }
  }
  int blocks_per_grid = num_threads / threads_per_block;

  perm_kernel<<<blocks_per_grid, threads_per_block>>>(cost_matrix_d, min_matrix_d, num_cities, num_threads);

  //copy local mins back to host
  cudaError_t rtn = cudaMemcpy(min_matrix_h, min_matrix_d, min_matrix_size, cudaMemcpyDeviceToHost);
  if(rtn != 0){
      printf("Ouchie:\n%s\n", cudaGetErrorString(rtn));
  }
    

  //calculate minimum
  int shortest_length = INT_MAX;
  for(int i = 0; i < num_threads; i++){
      if(min_matrix_h[i] < shortest_length)
      {
          shortest_length = min_matrix_h[i];
      }
  }

  double stop = now();
  printf("Shortest %d\n", shortest_length);
  printf("Blocks per grid:%d\n", blocks_per_grid);
  printf("Threads per block:%d\n", threads_per_block);
  printf("Num cities:%d\n", num_cities);
  printf("Num threads:%d\n", num_threads);
  printf("Took %5.3f seconds\n\n", stop - start);
}

