README for Homework 7
----------------------

Name: Jack Sampson
Assignment: Homework 7

Introduction:
-------------
This README details the implementation of the final project for CS 355, a VFS library implementing FAT-16 Disk. The Shell should call commands that utilize file system functions to interface with the disk via the VFS Library.

How to Compile:
---------------
This project is compiled using a provided Makefile. To compile the program, navigate to the source code directory and type make all in the terminal.

The Makefile configuration:
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS =

To compile, run:
`make`

To clean compiled files, run:
`make clean`

Test:
-----
1. build the project using the `make` command
2. Install hexedit and mtools for debugging
3. First make the FAT 16 DISK by typing `./bin/format NEWDISK -s 2` (makes a 2MB FAT-16 DISK in directory)
3. you can check the filesystem by typing `mdir -i NEWDISK` and move files in/out by `mcopy -i DISK filename.txt ::FILENAME.TXT` and the entire contents using `hexedit NEWDISK`
4. Test the fat16_write function with `./bin/test_vfs_write NEWDISK`
5. Test the fat16_read function with `./bin/test_vfs_read NEWDISK`
6. Test the fat16_stat function with `./bin/test_vfs_filestat NEWDISK <filename>`
7. Test the fat16_opendir, fat16_readdir, fat16_closedir functions with `./bin/test_vfs_lsroot NEWDISK`
8. Test the fat16_remove function with `./bin/test_vfs_rootclr NEWDISK` (Should clear the files from disk)
9. Type `mdir -i NEWDISK`, if there's an issue, type `rm NEWDISK` and recreate it with step 3.
10. Test the fat16_mkdir, fat16_close, fat16_rmdir with `./bin/test_vfs_dircycle NEWDISK` (makes a folder, writes a file to folder, and deletes both the file and folder).
11. delete the disk by typing `rm NEWDISK` and recreate it with step 3.
12. Test the command line by doing step 4 to create files and then typing `./bin/mysh NEWDISK` and typing `ls` in the shell.

Features:
---------
File system functions working as requested and can cooperate with Mtools to write and read from contents in the filesystem. The goal was to have a good amount of capabilities to the move to commands. Some commands can be useed by ./bin/mysh with the file system but it needs more definition. The code is simple enough to validate proper execution.

Known Bugs and Limitations:
---------------------------
The fat16_write function doesnt store the metadata (creation timestamp)
The fat16_remove function doesnt rebuild the FAT Entry table so it clears everything
Theres several bug when it comes to larger files (writing multi-cluster files)
The command line function integration is yet to be completed
A lot of variables were not used like mode and attribute capabilities (Readonly, Truncate)

File Directory:
---------------
bin/            - Compiled Apps
include/        - Header Files
src/            - Source C Files
test/           - Test Files

Notes:
------
This project was very indepth, I had to read a lot of spec sheets about FAT16 file system and try to get the functionality bug-tested using MTools. There was very little proper testing capabilities but I found that I can compare with a hex editor to see if my output matches MTools. Theres a lot more I wanted to accomplish but the file system errors kept me from getting to the higher level integration with the shell, however i did make it so the ./bin/mysh can call the ls and cat functions that do use the filesystem. With more time I could end up implementing the rest of the shell functions.

End of README 
