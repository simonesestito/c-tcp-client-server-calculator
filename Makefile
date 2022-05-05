CC := gcc
CFLAGS := -Wall -O2 -pthread

COMMON_DIR := common
COMMON_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMMON_OBJ := $(COMMON_SRC:.c=.o)

SERVER_DIR := server
SERVER_SRC := $(wildcard $(SERVER_DIR)/*.c)
SERVER_OBJ := $(SERVER_SRC:.c=.o)
SERVER_EXEC := server.out

CLIENT_DIR := client
CLIENT_SRC := $(wildcard $(CLIENT_DIR)/*.c)
CLIENT_OBJ := $(CLIENT_SRC:.c=.o)
CLIENT_EXEC := client.out

SUBPROJECTS := $(COMMON_DIR) $(SERVER_DIR) $(CLIENT_DIR)

.PHONY: softclean

all: $(SUBPROJECTS)

$(COMMON_DIR): $(COMMON_OBJ)

$(SERVER_DIR): $(SERVER_EXEC)

$(SERVER_EXEC): $(COMMON_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(COMMON_OBJ) -o $@

$(CLIENT_DIR): $(CLIENT_EXEC)

$(CLIENT_EXEC): $(COMMON_OBJ) $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) $(COMMON_OBJ) -o $@


%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.c: %.h

%.h:

softclean:
	rm -f $(CLIENT_OBJ)
	rm -f $(SERVER_OBJ)
	rm -f $(COMMON_OBJ)

clean: softclean
	rm -f $(CLIENT_EXEC)
	rm -f $(SERVER_EXEC)
	rm -f $(wildcard *.log)
