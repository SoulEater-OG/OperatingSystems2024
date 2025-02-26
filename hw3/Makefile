# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
TEST_DIR = test
BIN_DIR = bin

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Executable name
EXEC = $(BIN_DIR)/mysh

# Default target
all: $(EXEC)

# Ensure bin and obj directories exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Link object files to create executable
$(EXEC): $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter-out $(OBJ_DIR)/test_%.o, $(OBJ_FILES))

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean
