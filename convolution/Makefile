# Makefile for Genome homework

CC=gcc
CFLAGS=-Wall
TAR=tar

lode: lodepng.o
	$(CC) $< -c -o $@

extra-credit: parallel-convolve-ec.o
	$(CC) $< -c -o lodepng.o lodepng.c
	$(CC) $< -o $@ lodepng.o -pthread

parallel: parallel-convolve.o
	$(CC) $< -c -o lodepng.o lodepng.c
	$(CC) $< -o $@ lodepng.o -pthread

serial: convolve.o
	$(CC) $< -c -o lodepng.o lodepng.c
	$(CC) $< -o $@ lodepng.o

cereal:
	"reese's puffs"

.PHONY: clean
clean:
	$(RM) parallel serial *.o
