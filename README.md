README for Homework 4
----------------------

Your name: Jack Sampson
Assignment: Homework 4

Introduction:
-------------
This README details the implementation of HW4, in which a user thread library was designed (libuserthread.so) and tested with FIFO/SJF/PRIORITY scheduling.

How to Compile:
---------------
This project is compiled using a provided Makefile. To compile the program, navigate to the source code directory and type make all in the terminal. This compiles the library and libuserthread.so.

The Makefile configuration:
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -fpic -pthread
LDFLAGS = -shared -pthread

To compile, run:
make all

To clean compiled files, run:
make clean

How to Run:
-----------
The library doesnt run by itself but then compiles all the tests that can run using:
make test

alternatively valgrind can be done on the running of test by typing:
VALGRIND=1 make test
get a file printout of the valgrind memcheck in valgrind-out-<name of compiled test>.txt

Features:
---------
Priority/SJF/FIFO Scheduling: multiple ways to schedule function runs.

Known Bugs and Limitations:
---------------------------
It cannot do a illegal thread id in join because logic uses the join to schedule the threads. The thread_create adds them to the stack but they are pending the scheduler run on join. This also causes ripple effects on some test cases noteably the non-linear joins, and some misuse (test_misuse). There is also a prolime with priority queues does not schedule enough times.

File Directory:
---------------
src/
userthread.c: 
include/
userthread.h: Supplied userthread header file

Notes:
------
I updated the logic so it is mainly driving by the thread_join function to call out the condition signal to start the scheduler instead of the thread_create. This solved a lot of early problems where threads shouldn't be running and were, as well as when there was context switching back to main context. 

End of README 
