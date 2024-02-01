# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -fopenmp -I./header

# Source and header directories
SRC_DIR = src
HEADER_DIR = header

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=%.o)

# Executable name
TARGET = cifre

# Build rule
all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Object file rule
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ_FILES) $(TARGET)
