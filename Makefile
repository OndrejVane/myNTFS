#
# Makefile myNTFS
# Author: Ondrej Vane
# Date: 26.12.2018
#

CC = gcc
C99 = -std=c99

myNTFS : main.o file.o shell.o global.o shell_function.o mft_tree.o
	$(CC) main.o file.o shell.o global.o shell_function.o mft_tree.o -o myNTFS
	rm main.o file.o shell.o global.o shell_function.o mft_tree.o

main.o : main.c
	$(CC) $(C99) -c main.c

file.o : file.c
	$(CC) $(C99) -c -w file.c

shell.o : shell.c
	$(CC) $(C99) -c -w shell.c

global.o : global.c
	$(CC) $(C99) -c global.c

shell_function.o : shell_function.c
	$(CC) $(C99) -c shell_function.c

mft_tree.o : mft_tree.c
	$(CC) $(C99) -c mft_tree.c


clean :
	rm main.o file.o shell.o global.o shell_function.o mft_tree.o