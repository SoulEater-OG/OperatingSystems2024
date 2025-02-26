CC=gcc
CFLAGS=-Wall -g
LIBS=-lpthread
LIBS2=-lreadline

all: matrix mysh

matrix: matrix.c matrix_operations.c
	$(CC) $(CFLAGS) -o matrix matrix.c matrix_operations.c $(LIBS)
mysh: mysh.o
	$(CC) $(CFLAGS) -o mysh mysh.o $(LIBS2)
mysh.o: mysh.c
	$(CC) $(CFLAGS) -c mysh.c
clean:
	rm -f matrix mysh *.o

