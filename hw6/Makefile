CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SOURCES = defrag.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = defrag

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
