README for Homework 3
----------------------

Your name: Jack Sampson
Assignment: Homework 3

Introduction:
-------------
This README details the implementation of HW3, in which an extended shell (mysh) with job control was designed. The shell supports background execution, job control commands (fg, bg, jobs, kill), command chaining with ;, file redirection (>, <), and piping (|).

How to Compile:
---------------
This project is compiled using a provided Makefile. To compile the program, navigate to the source code directory and type make in the terminal. This compiles the shell executable mysh.

The Makefile configuration flags:
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

To compile, run:
make all

To clean compiled files, run:
make clean

How to Run:
-----------
The shell can be executed by running the mysh executable located in the bin directory:
./bin/mysh

Features:
---------
Background Execution: Run commands in the background using &.
Job Control: Commands like fg, bg, jobs, and kill to manage jobs.
Process Groups: Each command runs in its own process group.
Terminal Management: Proper management of terminal access for foreground jobs.
Command Chaining: Execute multiple commands separated by ;.
File Redirection: Input and output redirection using < and >.
Piping: Connect the output of one command to the input of another using |.

Known Bugs and Limitations:
---------------------------
Commands involving complex combinations of redirection and piping might not handle all edge cases.
There may be issues with handling certain special characters and excessive whitespace in commands.
Some built-in commands may not behave exactly like their counterparts in traditional shells.

File Directory:
---------------
src/            Directory where the source build files are located.
job_control.c:  Implementation of job control functions.
main.c:         Entry point of the shell program.
parser.c:       Command parsing and splitting functions.
shell.c:        Core shell functionality and command execution.

include/        Directory where the library header files are located.
job_control.h:  Header file for job control functions.
parser.h:       Header file for command parsing functions.
shell.h:        Header file for core shell functionality.

bin/:           Directory where the compiled shell executable mysh is located.
obj/:           Directory where the compiled object files are located.

Tests:
------

1. Basic Command Execution:
echo "hello world"
ls
pwd

2. Background Execution:
sleep 5 &
echo "background job"

3. Job Control:
sleep 10 &
jobs
fg %1
sleep 10 &
bg %1

4.Signal Handling (Control-Z):
sleep 10 (Press Control-Z during execution)
jobs
bg 1
fg 1

5. Built-in Commands:
jobs
kill 1
bg 1
fg 1

6. Process Groups and Terminal Management:
sleep 10 &
fg %1

7. Command Chaining with ;:
echo "hello"; echo "world"
ls; pwd; echo "done"

8. Redirection:
echo "hello world" > test.txt
cat test.txt
ls > out.txt
cat < out.txt

9. Piping:
ls -l | grep txt
cat test.txt | wc -l
echo "line1" > file.txt; echo "line2" >> file.txt; cat file.txt | grep line1

10. Combination of Features:
ls -l | grep test > out.txt &
cat out.txt

11. Error Handling:
invalid commands
cat non_existent_file.txt
ls > /invalid/path/out.txt

12. Special Characters and Whitespaces:
echo "test" > test.txt
cat < test.txt
ls -l | grep test

Notes:
------
The shell uses signal handlers to manage job control and terminal access. Signals such as SIGCHLD and SIGTSTP are handled to manage child processes and job suspension.
Redirection and piping are handled by modifying the file descriptors before executing commands.
The shell supports multiple built-in commands (jobs, kill, fg, bg) to control background and suspended jobs.

End of README 
