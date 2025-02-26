CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -fpic
LDFLAGS = -shared
INCLUDE_DIR = ./include
SRC_DIR = ./src
TEST_DIR = ./test
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
LIB = libmem.so
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TESTS = $(patsubst $(TEST_DIR)/%.c,$(TEST_DIR)/%,$(TEST_SRCS))

.PHONY: all clean test

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

$(TEST_DIR)/%: $(TEST_DIR)/%.c $(LIB)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $< -L. -lmem

test: $(TESTS)
	for t in $(TESTS); do \
		LD_LIBRARY_PATH=. ./$$t; \
	done

clean:
	rm -f $(OBJS) $(LIB) $(TESTS)