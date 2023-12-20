# Compiler
CC = gcc

HEADER_DIR = ../header
TEST_DIR = ../unit_test

# Compiler flags
CFLAGS = -Wall -Wextra -I$(HEADER_DIR) -I$(TEST_DIR)

# Source and header directories
SRC_DIR = ../src

# Source files
SRC_FILES = $(wildcard $(TEST_DIR)/*.c) $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c)) 

# Object files
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=%.o)

# Executable name
TARGET = test_cifre

# Build rule
all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Object file rule
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f ./*.o $(TARGET)
