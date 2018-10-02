#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mat-io.h"

#define MAT_ELT(mat, cols, i, j) *(mat + (i * cols) + j)

void usage(char *prog_name)
{
    fprintf(stderr, "%s: -a <filename> -b <filename> -o <filename> [-h]\n", prog_name);
    fprintf(stderr, "  -a   The name of the first matrix input file\n");
    fprintf(stderr, "  -b   The name of the second matrix input file\n");
    fprintf(stderr, "  -o   The name of the output file\n");
    fprintf(stderr, "  -h   Prints the usage\n");
    exit(1);
}


void
mat_print(char *msge, int *a, int m, int n) 
{
    printf("\n== %s ==\n%7s", msge, "");
    for (int j = 0;  j < n;  j++) {
        printf("%6d|", j);
    }
    printf("\n");

    for (int i = 0;  i < m;  i++) {
        printf("%5d|", i);
        for (int j = 0;  j < n;  j++) {
            printf("%7d", MAT_ELT(a, n, i, j));
        }
        printf("\n");
    }
}

void
mat_mult(int *c, int *a, int *b, int m, int n, int p)
{
  for (int i = 0;  i < m;  i++) {
    for (int j = 0;  j < p;  j++) {
      for (int k = 0;  k < n;  k++) {
        MAT_ELT(c, p, i, j) += MAT_ELT(a, n, i, k) * MAT_ELT(b, p, k, j);
      }
    }
  }
}

int
main(int argc, char **argv)
{
    char *prog_name = argv[0];
    if(argc < 4) {
        usage(prog_name);
    }

    int ch;
    char *a_file, *b_file, *o_file;
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

    int *a, *b, *c;
    int m, n, p;

    read_matrix(a, a_file);
    read_matrix(b, b_file);


    mat_mult((int *)c, (int *)a, (int *)b, m, n, p);
    mat_print("A", (int *)a, m, n);
    mat_print("B", (int *)b, n, p);
    mat_print("C", (int *)c, m, p);

    free(a);
    free(b);
    free(c);
}
