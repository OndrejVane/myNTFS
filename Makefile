#
# Makefile myNTFS
# Author: Ondrej Vane
# Date: 26.12.2018
#

CC = gcc
C99 = -std=c99

myNTFS : main.o file.o shell.o
	$(CC) main.o file.o shell.o -o myNTFS
	rm main.o file.o shell.o

main.o : main.c
	$(CC) $(C99) -c main.c

file.o : file.c
	$(CC) $(C99) -c file.c

shell.o : shell.c
	$(CC) $(C99) -c shell.c

clean :
	rm main.o file.o shell.o