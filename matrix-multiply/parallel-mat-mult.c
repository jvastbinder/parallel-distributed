#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

#include "mat-io.h"

#define ONE_BILLION (double)1000000000.0

double
now(void)
{
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time); return current_time.tv_sec + (current_time.tv_nsec / ONE_BILLION);
}

void usage(char *prog_name) {
    fprintf(stderr, "%s: -a <filename> -b <filename> -o <filename> [-h]\n", prog_name);
    fprintf(stderr, "  -a   The name of the first matrix input file\n");
    fprintf(stderr, "  -b   The name of the second matrix input file\n");
    fprintf(stderr, "  -o   The name of the output file\n");
    fprintf(stderr, "  -h   Prints the usage\n");
    exit(1);
}

void
mat_print(int tid, int *matrix, int total, int c) {
    printf("tid: %d\n", tid);
    for(int i = 0; i < total; i++) {
        printf("%d ", matrix[i]);
        if((i > 0) && (i % c == (c - 1)))
            printf("\n");
    }
}

void distribute_matrices(int num_threads, char *a_file, char *b_file, int m, int n, int p,
                         int *a_part, int *b_part) {
    int *a = malloc(sizeof(int) * m * n);
    int *b = malloc(sizeof(int) * n * p);
    read_matrix(a, a_file);
    read_matrix(b, b_file);
    int portion_of_a = ((m / num_threads) * n) + (m * n * (m % num_threads));
    int portion_of_b = ((p / (num_threads)) * n) + (n * p * (p % num_threads));

    for(int i = 0; i < num_threads; i++) {
        for(int j = i * ((m * n) / num_threads); j < ((m * n) / num_threads) + (((m * n) / num_threads) * i); j++) {
            a_part[j % ((m * n) / num_threads)] = a[j];
        }
        if(i < num_threads - 1)
            MPI_Send((void *)a_part, portion_of_a, MPI_INT, num_threads - 1 - i, 1, MPI_COMM_WORLD);
    }

    int tmp, k_start, k_bound, b_part_idx;
    for(int i = 0; i < num_threads; i++) {
        tmp = 0;
        for(int j = 0; j < n; j++) {
            k_start = (i * (p / num_threads));
            k_bound = (i * (p / num_threads)) + (p / num_threads);
            for(int k = k_start; k < k_bound; k++) {
                b_part_idx = ((k - k_start) * p) + j;
                b_part[b_part_idx] = b[(j * p) + k];
                tmp++;
            }
        }
        if(i < num_threads - 1)
            MPI_Send((void *)b_part, portion_of_b, MPI_INT, num_threads - 1 - i, 1, MPI_COMM_WORLD);
    }
}
int calc_next_c_idx(int big_i, int tid, int num_threads, int m, int n, int p) {
    int idx, c_section;
    c_section = (n * p) / num_threads;
    idx = big_i * (c_section / num_threads);
    return idx;
}

void compute_section(int *c, int big_i, int tid, int num_threads, int *a, int *b, int m, int n, int p) {
    int x, y, c_idx;
    c_idx = calc_next_c_idx(big_i, tid, num_threads, m, n, p);
    for(int i = 0; i < m/num_threads; i++) {
        for(int j = 0; j < n/num_threads; j++) {
            c[c_idx] = 0;
            for(int k = 0; k < p; k++) {
                x = a[(i * n) + k];
                y = b[(j * p) + k];
                }
                c[c_idx] += x * y;
            }
        c_idx++;
    }
}

int main(int argc, char ** argv) {
    int start = now();
    MPI_Init(&argc, &argv);

    char *prog_name = argv[0];
    if(argc < 7) {
        usage(prog_name);
    }

    int num_threads, tid;
    MPI_Comm_size(MPI_COMM_WORLD, &num_threads);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);

    int *mnp = malloc(sizeof(int) * 3);
    char *a_file, *b_file, *o_file;
    if(tid == 0) {
    int ch;
    while ((ch = getopt(argc, argv, "a:b:o:h")) != -1) {
        switch (ch) {
            case 'a':
                a_file = optarg;
                break;
            case 'b':
                b_file = optarg;
                break;
            case 'o':
                o_file = optarg;
                break;
            case 'h':
            default:
                usage(prog_name);
        }
    }
    argc -= optind;
    argv += optind;
    read_dimensions(&mnp[0], &mnp[1], a_file);
    read_dimensions(&mnp[1], &mnp[2], b_file);
    }
    MPI_Bcast((void *)mnp, 3, MPI_INT, 0, MPI_COMM_WORLD);

    int m = mnp[0];
    int n = mnp[1];
    int p = mnp[2];
    free(mnp);

    int portion_of_a = ((m / num_threads) * n) + (m * n * (m % num_threads));
    int portion_of_b = ((p / (num_threads)) * n) + (n * p * (p % num_threads));
    int portion_of_c = ((m / (num_threads)) * p) + (m * p * (m % num_threads));

    int *a_part = malloc((sizeof(int) * portion_of_a));
    int *b_part = malloc((sizeof(int) * portion_of_b));
    int *c_part = malloc((sizeof(int) * portion_of_c));

    if(tid == 0) {
        distribute_matrices(num_threads, a_file, b_file, m, n, p, a_part, b_part);
    }
    else{
        MPI_Recv((void *)a_part, portion_of_a, MPI_INT, 0, 1, MPI_COMM_WORLD, NULL);
        MPI_Recv((void *)b_part, portion_of_b, MPI_INT, 0, 1, MPI_COMM_WORLD, NULL);
    }

    int *a_part_old = malloc((sizeof(int) * portion_of_a));
    int *b_part_old = malloc((sizeof(int) * portion_of_b));
    int next_thread = tid;
    int prev_thread = tid;
    for(int i = 0; i < num_threads; i++){
        memcpy(a_part_old, a_part, portion_of_a * sizeof(int));
        memcpy(b_part_old, b_part, portion_of_b * sizeof(int));
        compute_section(c_part, i, tid, num_threads, a_part, b_part, m, n, p);
        next_thread++;
        if(next_thread == num_threads)
            next_thread = 0;
        prev_thread--;
        if(prev_thread == -1)
            prev_thread = num_threads - 1;
        MPI_Sendrecv(a_part_old, portion_of_a, MPI_INT, next_thread, 1,
                     a_part, portion_of_a, MPI_INT, prev_thread, 1,
                     MPI_COMM_WORLD, NULL);
        MPI_Sendrecv(b_part_old, portion_of_b, MPI_INT, next_thread, 1,
                     b_part, portion_of_b, MPI_INT, prev_thread, 1,
                     MPI_COMM_WORLD, NULL);
    }

    if(tid != 0){
        MPI_Send((void *)c_part, portion_of_c, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
    else{
        int *c = malloc(sizeof(int) * m * p);
        for(int i = 1; i < num_threads; i++) {
            MPI_Recv((void *)c_part, portion_of_c, MPI_INT, i, 1, MPI_COMM_WORLD, NULL);
            memcpy(&c[(i * portion_of_c) - 1], c_part, (sizeof(int) * portion_of_c));
        }
        memcpy(&c[(0 * portion_of_c) - 1], c_part, (sizeof(int) * portion_of_c));
        write_matrix(c, o_file, n, p);
//        free(c);
    }
 //   free(a_part);
  //  free(b_part);
   // free(c_part);

    MPI_Finalize();
    printf("took: %f seconds\n", now() - start);
}

