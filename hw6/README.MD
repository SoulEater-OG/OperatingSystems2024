README for Homework 6
----------------------

Your name: Jack Sampson
Assignment: Homework 6

Introduction:
-------------
This README details the implementation of HW6, in which a defragmentation tool (defrag) was designed and tested. The defragmentation tool is for Unix-like (inode-based) file system. The defragmentation tool improves filesystem performance by compacting all the blocks of a file into sequiential order on disk. The project provides a command-line tool (defrag) that reads a fragmented disk image, defragments the files and writes out a new disk image with the defragmented files.

How to Compile:
---------------
This project is compiled using a Makefile. To compile the program, navigate to the source code directory and run the following command in the terminal:

The Makefile configuration:
CC=gcc
CFLAGS=-I./include

To compile, run:
make

To clean compiled files, run:
make clean

How to Run:
-----------
To run the defragmenter, use the following command:
./defrag <path to fragmented disk file>
The defragmenter will read the specified fragmented disk file, defragment the files, and output a new disk image with "-defrag" concatenated to the end of the input file name.

If you want to check the memory you can type:
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose . /defrag <path to fragmented disk file>

Features:
---------
Reads a fragmented disk image containing a Unix-like file system
Compacts all the blocks of a file into sequential order on disk
Outputs a new disk image with the defragmented files
Preserves the original boot block
Updates the superblock with a new list of free blocks (sorted from lowest to highest)
Updates the inodes for the defragmented files
Relocates the data blocks to their new locations

Known Bugs and Limitations:
---------------------------
The tool has been thoroughly tested and currently has no known bugs or memory leaks.
Handles all types of direct, indirect, doubly indirect, and triply indirect blocks effectively.

File Directory:
---------------
defrag.c: Implementation of the defragmenter
defrag.h: Header file for the defragmenter

Notes:
------
The defragmentation tool now can handle all types of blocks, including double and triple indirect blocks, as well as direct and empty blocks, without error or memory leak.

End of README
