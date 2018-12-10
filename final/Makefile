# Makefile for Genome homework

CC=gcc
CFLAGS=-Wall
TAR=tar

parallel: merge-sort-threaded.o
	$(CC) $< -o $@ -lz -pthread

