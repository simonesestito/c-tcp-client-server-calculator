SERVER_DIR := server
SERVER_SRC := $(wildcard $SERVER_DIR/*.c $SERVER_DIR/Makefile $SERVER_DIR/*.h)

CLIENT_DIR := client
CLIENT_SRC := $(wildcard $CLIENT_DIR/*.c $CLIENT_DIR/Makefile $CLIENT_DIR/*.h)

SUBPROJECTS := $(SERVER_DIR) $(CLIENT_DIR)

.PHONY: $(SUBPROJECTS) clean

all: $(SUBPROJECTS)

$(SERVER_DIR): $(SERVER_SRC)
	make -C $@

$(CLIENT_DIR): $(CLIENT_SRC)
	make -C $@

clean:
	make -C $(SERVER_DIR) clean
	make -C $(CLIENT_DIR) clean
