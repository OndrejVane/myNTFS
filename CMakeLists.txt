cmake_minimum_required(VERSION 3.10)
project(myNTFS C)

set(CMAKE_C_STANDARD 99)

add_executable(myNTFS main.c file.c file.h structures.h shell.c shell.h)