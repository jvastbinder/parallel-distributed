# Makefile for Genome homework

CC=gcc
CFLAGS=-Wall
TAR=tar

generator: mat-gen.o 
	$(CC) $< -c -o mat-io.o mat-io.c
	$(CC) $< -o $@ mat-io.o 

io: mat-io.o
	$(CC) $< -c -o $@

parallel: parallel-convolve.o
	$(CC) $< -c -o lodepng.o lodepng.c
	$(CC) $< -o $@ lodepng.o -pthread

serial: mat-mult.o
	$(CC) $< -o $@

cereal:
	"reese's puffs"

.PHONY: clean
clean:
	$(RM) parallel serial *.o
