CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -std=c11 -D_GNU_SOURCE
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include
TEST_DIR = tests

# Source files - all .c files in src/
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

# Main executable target
TARGET = $(BIN_DIR)/clhfm

# Test files
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_TARGET = $(BUILD_DIR)/test_runner

# Default target
all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Build successful: $(TARGET)"

install: $(TARGET)
	@echo "Installing C-LHF&D-M..."
	@sudo mkdir -p /usr/local/bin
	@sudo cp $(TARGET) /usr/local/bin/clhfm
	@sudo chmod +x /usr/local/bin/clhfm
	@echo "Installation complete! Use 'clhfm help' to get started."

uninstall:
	@echo "Uninstalling C-LHF&D-M..."
	@sudo rm -f /usr/local/bin/clhfm
	@echo "Uninstallation complete!"

test: $(TEST_SOURCES)
	@echo "Building and running tests..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SOURCES) $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) $(LDFLAGS) -o $(TEST_TARGET)
	@$(TEST_TARGET)

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

.PHONY: all install uninstall test clean
