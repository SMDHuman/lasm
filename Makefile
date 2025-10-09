# Source directories
SRC_DIR := src
CPU_DIR := src/hardware
BUILD_DIR := build
INSTALL_DIR := /usr/local/bin

# Compiler and flags
CC := cc
CFLAGS := -ggdb -Wall -Wextra -I include -I $(CPU_DIR) -I $(SRC_DIR)

# Object files
OBJECTS := $(BUILD_DIR)/lasm_assembler.o \
			 $(BUILD_DIR)/lasm_parser.o \
			 $(BUILD_DIR)/lasm_macro.o \
			 $(BUILD_DIR)/lasm_tokenizer.o

# Main target
$(BUILD_DIR)/lasm: $(BUILD_DIR) $(wildcard $(SRC_DIR)/*) $(wildcard $(CPU_DIR)/*) $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(SRC_DIR)/lasm.c $(CFLAGS)

# Generic rule for object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h
	$(CC) -c $< -o $@ $(CFLAGS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $@

# Phony targets
.PHONY: clean examples

all: clean $(BUILD_DIR)/lasm 

clean:
	rm -rf $(BUILD_DIR)

install: all
	touch $(INSTALL_DIR)
	cp $(BUILD_DIR)/lasm $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/lasm

# Examples
examples: example_basic_syntax example_expressions example_namespaces example_fibonacci example_6502_addressing

example_%: $(BUILD_DIR)/lasm examples/%.l
	./$(BUILD_DIR)/lasm examples/$*.l -m 6502 -o $(BUILD_DIR)/$*.out
