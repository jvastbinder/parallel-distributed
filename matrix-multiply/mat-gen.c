#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mat-io.h"

void
usage(char *prog_name)
{
    fprintf(stderr, "%s: -o <filename> -m <rows> -n <cols> [-h]\n", prog_name);
    fprintf(stderr, "  -o   The name of the output file\n");
    fprintf(stderr, "  -m   The number of rows in the matrix\n");
    fprintf(stderr, "  -n   The number of columns in the matrix\n");
    fprintf(stderr, "  -h   Prints the usage\n");
    exit(1);
}

void gen_matrix(int* matrix, int m, int n){
    for(int i = 0; i < (m * n); i++) {
            matrix[i] = rand();
        }
}

int
main(int argc, char **argv)
{
    char *prog_name = argv[0];
    if(argc < 4) {
        usage(prog_name);
    }

    int ch, m, n;
    char *output_file;
    while ((ch = getopt(argc, argv, "o:m:n:h")) != -1) {
        switch (ch) {
            case 'o':
                output_file = optarg;
                break;
            case 'm':
                m = atol(optarg);
                break;
            case 'n':
                n = atol(optarg);
                break;
            case 'h':
            default:
                usage(prog_name);
        }
    }
    argc -= optind;
    argv += optind;

    int *matrix = malloc(sizeof(int) * m * n);
    gen_matrix(matrix, m, n);
    write_matrix(matrix, output_file, m, n);
    free(matrix);

    printf("Output file: %s\n", output_file);
    printf("M: %d\n", m);
    printf("N: %d\n", n);
}
