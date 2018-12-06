/* Threaded merge sort.

   Example invocation: 
   ./merge-sort-threaded -s 50055 -n 25 -t 5 -v -v

   A rather clunky early version of this code appears at 
https://www.geeksforgeeks.org/merge-sort-using-multi-threading/
*/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int VERBOSE = 0;

/* ==== Debugging Output ================ */

/* Print `arr`, and array of length `len`. */
    void
print_array(char *msge, int *arr, int len)
{
    printf("%s\n", msge);
    for (int i = 0;  i < len;  i++) {
        printf("%s%d", i == 0 ? "" : " ", arr[i]);
    }
    putchar('\n');
}

/* Print an an array with an indicate of the locations of `low` and `high`. */
    void
print_delimited_array(char *msge, int tid, int *arr, int num_elts, int low, int high)
{
    char buf[8192];
    int idx = 0;
    for (int i = 0;  i < num_elts; i++) {
        idx += sprintf(buf + idx, " %3d ", arr[i]);
    }
    buf[low * 5] = '[';
    buf[high * 5 + 4] = ']';
    buf[idx] = '\0';

    printf("%s %3d (%3d - %3d): %s\n", msge, tid, low, high, buf);
}

/* ==== Merge Sort ================ */

void merge(int *arr, int low, int mid, int high)
{
    int left_length = mid - low + 1;
    int right_length = high - mid;

    int *left = malloc(sizeof(int) * left_length);
    int *right = malloc(sizeof(int) * right_length);

    /* ---------------- */
    for (int i = 0;  i < left_length;  i++) {
        left[i] = arr[low + i];
    }

    for (int i = 0;  i < right_length; i++) {
        right[i] = arr[mid + i + 1];
    }

    /* ---------------- */
    int i = 0;
    int j = 0;
    int k = low;

    while (i < left_length && j < right_length) {
        if (left[i] <= right[j]) {
            arr[k++] = left[i++];
        } else {
            arr[k++] = right[j++];
        }
    }

    /* ---------------- */
    while (i < left_length) {
        arr[k++] = left[i++];
    }

    while (j < right_length) {
        arr[k++] = right[j++];
    }

    free(left);
    free(right);
}

/* ---------------- */
    void
merge_sort(int tid, int *arr, int num_elts, int low, int high)
{
    int mid = low + (high - low) / 2;

    if (VERBOSE >= 2) {
        print_delimited_array(" ", tid, arr, num_elts, low, high);
    }

    /* ---------------- */
    if (low < high) {
        merge_sort(tid, arr, num_elts, low, mid);
        merge_sort(tid, arr, num_elts, mid + 1, high);
        merge(arr, low, mid, high);
        if (VERBOSE >= 2) {
            print_delimited_array("<", tid, arr, num_elts, low, high);
        }
    }
}

/* ==== Threaded Merge Sort ================ */

/* ---------------- */
typedef struct {
    int tid;
    int num_threads;
    int num_elts;
    int *arr;
} thread_args_t;

/* ---------------- */
typedef struct {
    int tid;
    int elts_per_thread;
    int low;
    int mid;
    int high;
} thread_results_t;

/* ---------------- */
    void *
merge_sort_thread(void *in)
{
    thread_args_t *args = (thread_args_t *)in;
    int *arr = args->arr;

    /* ---------------- */
    int elts_per_thread = args->num_elts / args->num_threads;
    int low = elts_per_thread * args->tid;
    int high = elts_per_thread * (args->tid + 1) - 1;
    int mid = low + (high - low) / 2;

    if (VERBOSE >= 1) {
        printf("%2d: %8d elts: lo %8d mid %8d hi %8d\n",
                args->tid, high - low + 1, low, mid, high);
    }

    /* ---------------- */
    if (low < high) {
        merge_sort(args->tid, arr, args->num_elts, low, mid);
        merge_sort(args->tid, arr, args->num_elts, mid + 1, high);
        merge(arr, low, mid, high);
    }

    /* ---------------- */
    thread_results_t *results = malloc(sizeof(thread_results_t));
    results->tid = args->tid;
    results->elts_per_thread = elts_per_thread;
    results->low = low;
    results->mid = mid;
    results->high = high;

    return (void *)results;
}

/* ==== Utilities ================ */

/* What time is it? */
    double
now(void)
{
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    return current_time.tv_sec + (current_time.tv_nsec / 1000000000.0);
}

/* Verify that array `arr` with `num_elts` elements is in sorted order. */
    void
verify_sort(int *arr, int num_elts)
{
    int ok = 1;

    for (int i = 0;  i < num_elts - 1;  i++) {
        if (arr[i] > arr[i + 1]) {
            printf("NOT SORTED at index %d: %d > %d\n", i, arr[i], arr[i + 1]);
            ok = 0;
        }
    }

    if (ok) {
        if (VERBOSE >= 1) {
            printf("SORTED\n");
        }
    } else {
        exit(2);
    }
}

    void
usage(char *prog_name)
{
    fprintf(stderr, "usage: %s [flags]\n", prog_name);
    fprintf(stderr, "   -n <array size>\n");
    fprintf(stderr, "   -s <random number seed>\n");
    fprintf(stderr, "   -v       Verbose output (repeat for more verbosity) \n");
    fprintf(stderr, "   -t <num threads>\n");
    fprintf(stderr, "   -h      Get help\n");
    exit(1);
}

/* ==== Main ================================================================ */

    int
main(int argc, char **argv)
{
    double start_time, stop_time;
    int num_elts = 20;			/* Number of elements in array */
    int num_threads = 4;			/* Number of threads */
    int *arr = NULL;				/* The array itself */

    int ch;
    while ((ch = getopt(argc, argv, "n:s:t:vh")) != -1) {
        switch (ch) {
            case 'n':
                num_elts = atoi(optarg);
                break;
            case 's':
                srand(atoi(optarg));
                break;
            case 'v':
                VERBOSE++;
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
            default:
                usage(argv[0]);
        }
    }

    if (num_elts % num_threads != 0) {
        fprintf(stderr, "Sorry, array size (%d) not a multiple of thread count (%d)\n",
                num_elts, num_threads);
        exit(3);
    }

    printf("%d threads, %d elements\n", num_threads, num_elts);

    /* ---------------- */
    arr = malloc(num_elts * sizeof(int));
    for (int i = 0; i < num_elts; i++) {
        arr[i] = rand() % (num_elts * 2);
    }
    if (VERBOSE >= 1) {
        print_array("Original", arr, num_elts);
    }

    start_time = now();

    /* ---------------- */
    thread_args_t *thread_args = (thread_args_t *)malloc(num_threads * sizeof(thread_args_t));
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    for (int t = 0; t < num_threads; t++) {
        thread_args[t].tid = t;
        thread_args[t].num_threads = num_threads;
        thread_args[t].num_elts = num_elts;
        thread_args[t].arr = arr;

        pthread_create(&threads[t], NULL, merge_sort_thread, (void *)&thread_args[t]);
    }

    /* ---------------- */
    thread_results_t **thread_results = malloc(num_threads * sizeof(thread_results_t *));
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], (void **) &thread_results[t]);
        if (VERBOSE >= 2) {
            printf("%d: %d - %d - %d\n",
                    thread_results[t]->tid,
                    thread_results[t]->low,
                    thread_results[t]->mid,
                    thread_results[t]->high);
        }
    }

    /* ---------------- */
    for (int t = 1;  t < num_threads;  t++) {
        merge(arr, 0, thread_results[t]->low - 1, thread_results[t]->high);
        if (VERBOSE >= 2) {
            print_delimited_array(" ", -1, arr, num_elts, 0, thread_results[t]->high);
        }
    }

    stop_time = now();

    if (VERBOSE >= 1) {
        print_array("Result", arr, num_elts);
    }
    verify_sort(arr, num_elts);

    printf("t_p = %.6f ms\n", (stop_time - start_time) * 1000.0);
}

