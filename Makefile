# Compiler and flags
CC := cc
INCLUDE := -I include/ -I src/cpu/
CFLAGS := -ggdb -Wall -Wextra

# Source directories
SRC_DIR := src
INCLUDE_DIR := include
CPU_DIR := src/cpu
BUILD_DIR := build

# Object files
OBJECTS := $(BUILD_DIR)/lasm_namespace.o \
			 $(BUILD_DIR)/lasm_assembler.o \
			 $(BUILD_DIR)/lasm_parser.o \
			 $(BUILD_DIR)/lasm_macro.o \
			 $(BUILD_DIR)/lasm_tokenizer.o

# Main target
$(BUILD_DIR)/lasm: $(BUILD_DIR) $(SRC_DIR)/lasm.c $(wildcard $(INCLUDE_DIR)/*) $(wildcard $(CPU_DIR)/*) $(OBJECTS)
	$(CC) -o $@ $(INCLUDE) $(OBJECTS) $(SRC_DIR)/lasm.c $(CFLAGS)

# Generic rule for object files
$(BUILD_DIR)/%.o: $(INCLUDE_DIR)/%.c $(INCLUDE_DIR)/%.h
	$(CC) -c $< -o $@ $(INCLUDE) $(CFLAGS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $@

# Phony targets
.PHONY: clean examples

all: clean $(BUILD_DIR)/lasm 

clean:
	rm -rf $(BUILD_DIR)

# Examples
examples: example_basics example_expressions example_namespaces example_fibonacci example_6502_addressing

example_%: $(BUILD_DIR)/lasm examples/%.l
	./$(BUILD_DIR)/lasm examples/$*.l -m 6502 -o $(BUILD_DIR)/$*.out
