README for Homework 1
----------------------

Your name: Jack Sampson
Assignment: Homework 1

How to Compile:
---------------
This project is compiled using a provided Makefile. To compile the programs, navigate to the directory containing the source code and type make in the terminal. This will compile the programs hw1a, hw1b, and microcat as specified in the Makefile.

The Makefile uses the following configuration:
Compiler: gcc
Compiler Flags: -Wall (for compiler warnings), -g (for debugging information)

Compiled executables hw1a, hw1b, and microcat will be created in the same directory.

To compile the files, you can run:
make all

To clean up the compiled files, you can run:
make clean

How to Run:
-----------
To run hw1a: ./hw1a
To run hw1b: ./hw1b
For a custom interval in seconds, use the -s flag: ./hw1b -s <interval>
To run microcat: ./microcat [file1] [file2] ...
If no files are specified, microcat will read from standard input.

Known Bugs and Limitations:
---------------------------
No known bugs

Summary of Features and Extra Credits Completed:
------------------------------------------------
EC not completed

File Directory:
---------------
hw1a.c: Source code for hw1a.
hw1b.c: Source code for hw1b.
microcat.c: Source code for microcat.
Makefile: Used for compiling the programs.

End of README
