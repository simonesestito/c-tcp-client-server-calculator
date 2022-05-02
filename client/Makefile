CC := gcc
CFLAGS := -Wall -O2 -lpthread
BUILD_DIR=build
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)
OBJECTS := $(addprefix $(BUILD_DIR)/,$(OBJECTS))
OUT_EXEC := ../client.out

all: $(BUILD_DIR) $(OUT_EXEC)

clean:
	rm -rf $(BUILD_DIR)/ $(OUT_EXEC) $(wildcard *.log)

$(OUT_EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

build/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.c: %.h

%.h:

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
