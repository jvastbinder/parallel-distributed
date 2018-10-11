#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "mat-io.h"

int debug = 0;

void
usage(char *prog_name)
{
    fprintf(stderr, "%s: -o <filename> -m <rows> -n <cols> [-h]\n", prog_name);
    fprintf(stderr, "  -o   The name of the output file\n");
    fprintf(stderr, "  -r   The number of rows in the matrix\n");
    fprintf(stderr, "  -c   The number of columns in the matrix\n");
    fprintf(stderr, "  -h   Prints the usage\n");
    exit(1);
}

void gen_matrix(int* matrix, int r, int c){
    for(int i = 0; i < r; i++) {
        for(int j = 0; j < c; j++) {
            matrix[(i * c) + j] = rand() % 10;
        }
    }
}

int
main(int argc, char **argv)
{
    srand(time(0));
    char *prog_name = argv[0];
    if(argc < 4) {
        usage(prog_name);
    }

    int ch, r, c;
    char *output_file;
    while ((ch = getopt(argc, argv, "o:r:c:h")) != -1) {
        switch (ch) {
            case 'o':
                output_file = optarg;
                break;
            case 'r':
                r = atol(optarg);
                break;
            case 'c':
                c = atol(optarg);
                break;
            case 'h':
            default:
                usage(prog_name);
        }
    }
    argc -= optind;
    argv += optind;

    printf("Output file: %s\n", output_file);
    printf("R: %d\n", r);
    printf("C: %d\n", c);
    int *matrix = malloc(sizeof(int) * r * c);
    gen_matrix(matrix, r, c);
    write_matrix(matrix, output_file, r, c);

}

