#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mat-io.h"

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
mat_print(char *msge, int *matrix, int r, int c) 
{
    for(int i = 0; i < r; i++){
        for(int j = 0; j < c; j++){
            printf("%d ", matrix[(i * r) + j]);
        }
        printf("\n");
    }
}

void
mat_mult(int *c, int *a, int *b, int m, int n, int p)
{
    int x = 0;
    int y = 0;
    for (int i = 0;  i < m;  i++) {
        for (int j = 0;  j < p;  j++) {
            c[i * m + j] = 0;
            for (int k = 0;  k < n;  k++) {
                x = a[(i * m) + k];
                y = b[(k * n) + j];
                c[(i * m) + j] += x * y;
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

    int m, n, p;
    read_dimensions(&m, &n, a_file);
    read_dimensions(&n, &p, b_file);

    int *a = malloc(sizeof(int) * m * n);
    int *b = malloc(sizeof(int) * n * p);
    int *c = malloc(sizeof(int) * m * p);
    read_matrix(a, a_file);
    read_matrix(b, b_file);

    mat_print("C", a, m, n);
    mat_print("C", b, n, p);

    mat_mult(c, a, b, m, n, p);
    //mat_print("C", c, m, p);
    
    write_matrix(c, o_file, m, p);

}
