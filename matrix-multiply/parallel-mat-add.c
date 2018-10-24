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

void distribute_matrices(int num_threads, char *a_file, char *b_file, int r, int c, int *a_part, int *b_part) {
    int bound;
    FILE *a = fopen(a_file, "r");
    FILE *b = fopen(b_file, "r");
    fscanf(a, "%d %d\n", &bound, &bound);
    fscanf(b, "%d %d\n", &bound, &bound);

    for(int i = 0; i < num_threads; i++) {
        if(i < num_threads - 1)
            bound = ((r * c) / num_threads);
        else
            bound = ((r * c) / num_threads) + ((r * c) % num_threads);
        for(int j = 0; j < bound; j++) {
            fscanf(a, "%d ", &a_part[j]);
            fscanf(b, "%d ", &b_part[j]);
        }
        if(i < num_threads - 1) {
            MPI_Send((void *)a_part, bound, MPI_INT, num_threads - 1 - i, 1, MPI_COMM_WORLD);
            MPI_Send((void *)b_part, bound, MPI_INT, num_threads - 1 - i, 1, MPI_COMM_WORLD);
        }
    }
}

void compute_section(int *c, int *a, int *b, int rows, int cols, int tid, int num_threads) {
    int bound;
    if(tid == 0)
        bound = (rows * cols) - ((num_threads - 1) * ((rows * cols) / num_threads));
    else
        bound = ((rows * cols) / num_threads);
    for(int i = 0; i < bound; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char ** argv) {
    int start = now();
    MPI_Init(&argc, &argv);

    char *prog_name = argv[0];

    int num_threads, tid;
    MPI_Comm_size(MPI_COMM_WORLD, &num_threads);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);

    int *rc = malloc(sizeof(int) * 2);
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
    read_dimensions(&rc[0], &rc[1], a_file);
    }
    MPI_Bcast((void *)rc, 2, MPI_INT, 0, MPI_COMM_WORLD);

    int r = rc[0];
    int c = rc[1];
    free(rc);

    int zero_mat_size = (r * c) - ((num_threads - 1) * ((r * c) / num_threads));
    int mat_size = ((r * c) / num_threads);

    int *a_part = malloc((sizeof(int) * zero_mat_size));
    int *b_part = malloc((sizeof(int) * zero_mat_size));
    int *c_part = malloc((sizeof(int) * zero_mat_size));

    if(tid == 0) {
        distribute_matrices(num_threads, a_file, b_file, r, c, a_part, b_part);
    }
    else{
        MPI_Recv((void *)a_part, mat_size, MPI_INT, 0, 1, MPI_COMM_WORLD, NULL);
        MPI_Recv((void *)b_part, mat_size, MPI_INT, 0, 1, MPI_COMM_WORLD, NULL);
    }

    compute_section(c_part, a_part, b_part, r, c, tid, num_threads);

    if(tid != 0){
        MPI_Send((void *)c_part, mat_size, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
    else{
        int *c_whole = malloc(sizeof(int) * r * c);
        memcpy(&c_whole[(num_threads - 1) * ((r * c) / num_threads)], c_part, (sizeof(int) * zero_mat_size));
        for(int i = 1; i < num_threads; i++) {
            MPI_Recv((void *)c_part, mat_size, MPI_INT, i, 1, MPI_COMM_WORLD, NULL);
            memcpy(&c_whole[(i - 1) * ((r * c) / num_threads)], c_part, (sizeof(int) * mat_size));
        }
        write_matrix(c_whole, o_file, r, c);
        free(c_whole);
    }
    free(a_part);
    free(b_part);
    free(c_part);

    MPI_Finalize();
    printf("took: %f seconds\n", now() - start);
}

