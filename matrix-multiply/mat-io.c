#include <stdio.h>
#include <stdlib.h>

void read_dimensions(int *r, int *c, char* filename){
    FILE *input = fopen(filename, "r");
    fscanf(input, "%d %d\n", r, c);
    fclose(input);
}

void read_matrix(int *matrix, char* filename){
    FILE *input = fopen(filename, "r");
    int r = 0;
    int c = 0;
    fscanf(input, "%d %d\n", &r, &c);
    for(int i = 0; i < r; i++) {
        for(int j = 0; j < c; j++) {
            if(!fscanf(input, "%d ", &matrix[(i * c) + j])){
                break;
            }
        }
    }
    fclose(input);
}

void write_matrix(int *matrix, char *filename, int r, int c){
    FILE *output = fopen(filename, "w");
    fprintf(output, "%d %d\n", r, c);
    for(int i = 0; i < r; i++) {
        for(int j = 0; j < c; j++) {
            fprintf(output, "%d ", matrix[(i * c) + j]);
        }
        fprintf(output, "\n");
    }
    fclose(output);
}

