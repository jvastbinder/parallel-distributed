# Makefile for Genome homework

CC=gcc
CFLAGS=-Wall -g
TAR=tar

serial: tsp-serial.c
	$(CC) $< -g -o $@

parallel: tsp-parallel.cu
	/usr/local/cuda-9.0/bin/nvcc $< -g -o $@

cereal:
	"reese's puffs"

.PHONY: clean
clean:
	$(RM) parallel serial *.o
