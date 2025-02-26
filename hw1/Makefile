CC=gcc
CFLAGS=-Wall -g

all: hw1a hw1b microcat

hw1a: hw1a.o
	$(CC) $(CFLAGS) -o hw1a hw1a.o
hw1b: hw1b.o
	$(CC) $(CFLAGS) -o hw1b hw1b.o
microcat: microcat.o 
	$(CC) $(CFLAGS) -o microcat microcat.o
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean: 
	rm -f *.o hw1a hw1b microcat
