CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -pedantic -fpic -pthread -g
LDFLAGS = -shared -pthread
INCLUDE_DIR = ./include
SRC_DIR = ./src
TEST_DIR = ./test
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
LIB = libuserthread.so
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TESTS = $(patsubst $(TEST_DIR)/%.c,$(TEST_DIR)/%,$(TEST_SRCS))

.PHONY: all clean test

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

$(TEST_DIR)/%: $(TEST_DIR)/%.c $(LIB)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $< -L. -luserthread -lrt

test: $(TESTS)
ifdef VALGRIND
	for t in $(TESTS); do \
		LD_LIBRARY_PATH=. valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out-`basename $$t`.txt ./$$t; \
	done
else
	for t in $(TESTS); do \
		LD_LIBRARY_PATH=. ./$$t; \
	done
endif

clean:
	rm -f $(OBJS) $(LIB) $(TESTS)
