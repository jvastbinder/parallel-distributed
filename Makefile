# Makefile for Genome homework

CC=gcc
CFLAGS=-Wall
TAR=tar

parallel: parallel-genome-search.o
	$(CC) $< -o $@ -lz -g -pthread

sg: sg.o
	$(CC) $< -o $@ -lz

handout.tar.gz: Makefile sg.c get-data.sh genome-hw.pdf
	$(TAR) zcvf $@ $^

.PHONY: clean
clean:
	$(RM) parallel sg *.o *~ README.{tex,pdf} handout.tar.gz
