#include <stdio.h>

void read_matrix(int* matrix, char* filename){
    FILE *input = fopen(filename, "r");
    /*
    fprintf(input, "%d %d\n", m, n);
    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            fprintf(input, "%d ", matrix[(i * m) + j]);
        }
        fprintf(input, "\n");
    }
    fclose(input);
    */
}

void write_matrix(int* matrix, char* filename, int m, int n){
    FILE *output = fopen(filename, "w");
    fprintf(output, "%d %d\n", m, n);
    for(int i = 0; i < m; i++) {
        for(int j = 0; j < n; j++) {
            fprintf(output, "%d ", matrix[(i * m) + j]);
        }
        fprintf(output, "\n");
    }
    fclose(output);
}

