README for Homework 5
----------------------

Name: Jack Sampson
Assignment: Homework 5

Introduction:
-------------
This README details the implementation of HW5, in which a memory allocator library (libmem.so) was designed and tested. The memory allocator provides functionality for allocating and freeing memory chunks within a user-level process. Several test cases were also created to validate the functionality of the library.

How to Compile:
---------------
This project is compiled using a provided Makefile. To compile the program, navigate to the source code directory and type make all in the terminal. This compiles the library and generates libmem.so.

The Makefile configuration:
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -fpic
LDFLAGS = -shared

To compile, run:
make all

To clean compiled files, run:
make clean

How to Run:
-----------
The library doesnt run by itself but then compiles all the tests that can run using:
make test

alternatively each test can be individually compiled and run with valgrind using these commands:
gcc -Wall -Wextra -Werror -pedantic -fpic -I./include -o ./test/test_<testname> ./test/test_<testname>.c -L. -lmem
LD_LIBRARY_PATH=. valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./test/test_<testname>

Features:
---------
Worst-fit memory allocation policy
Coalescing of free memory blocks
8-byte alignment for allocated memory
Error handling for invalid arguments and out-of-memory situations

Known Bugs and Limitations:
---------------------------
The memory allocator has a fixed memory region size determined at initialization, which may limit the number and size of allocations that can be made.

File Directory:
---------------
src/
    mem.c: Implementation of the memory allocator functions
include/
    mem.h: Supplied memory allocator header file

Notes:
------
Implementing the memory allocator was challenging, particularly in handling edge cases and ensuring proper memory management. The coalescing of free memory blocks and the worst-fit allocation policy required careful consideration and testing.

End of README 
