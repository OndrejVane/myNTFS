#
# Makefile myNTFS
# Author: Ondrej Vane
# Date: 26.12.2018
#

CC = gcc
C99 = -std=c99

myNTFS : main.o
	$(CC) main.o -o myNTFS

main.o : main.c
	$(CC) $(C99) -c main.c

clean :
	rm main.o