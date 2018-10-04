#ifndef MAT_IO_H 
#define MAT_IO_H

void read_dimensions(int *r, int *c, char *filename);
void read_matrix(int *matrix, char *filename);
void write_matrix(int *matrix, char *filename, int m, int n);

#endif
