CC = gcc
CFLAGS = -Wall -Wextra -g -I.

TARGET ?= toarray

EXAMPLE_DIR = examples
OUTPUT_DIR = build

.PHONY: all run clean

run: $(TARGET)
	@echo "RUN  ==> ./$(OUTPUT_DIR)/$(TARGET)"
	./$(OUTPUT_DIR)/$(TARGET)

# $@ means the target (e.g., "toarray")
# $^ means all prerequisites (e.g., "toarray.o stream.o")
$(TARGET):
	@echo "CC ==> $@"
	$(CC) $(CFLAGS) $(EXAMPLE_DIR)/$@.c stream.c -o $(OUTPUT_DIR)/$(TARGET)

clean:
	@echo "CLEAN"
	rm -f $(OUTPUT_DIR)/*
