# Makefile for Genome homework

CC=gcc

serial: tsp-serial.c
	$(CC) $< -g -o $@

parallel: tsp-parallel.cu
	/usr/local/cuda-9.0/bin/nvcc $< -o $@

cereal:
	milk "reese's puffs"

.PHONY: clean
clean:
	$(RM) parallel serial *.o
