# Makefile for Genome homework

CC=gcc
CFLAGS=-Wall -g
TAR=tar

generator: mat-gen.o mat-io.o
	$(CC) $< -g -c -o mat-io.o mat-io.c
	$(CC) $< -g -o $@ mat-io.o 

io: mat-io.o
	$(CC) $< -g -c -o $@

serial: mat-mult.o mat-io.o
	$(CC) $< -g -c -o mat-io.o mat-io.c
	$(CC) $< -g -o $@ mat-io.o

parallel: parallel-mat-mult.o mat-io.o
	$(CC) $< -g -c -o mat-io.o mat-io.c
	mpicc $< -g -o $@ mat-io.o

extra-credit: parallel-mat-add.o mat-io.o
	$(CC) $< -g -c -o mat-io.o mat-io.c
	mpicc $< -g -o $@ mat-io.o

cereal:
	"reese's puffs"

.PHONY: clean
clean:
	$(RM) parallel serial *.o
