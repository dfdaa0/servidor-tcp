all: client server

CC = gcc
override CFLAGS += -g -Wno-everything -pthread -lm

SRC_DIR = src
CLIENT_SRC = $(SRC_DIR)/client.c
SERVER_SRC = $(SRC_DIR)/server.c

CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

BIN_DIR = bin
CLIENT_BIN_DIR = $(BIN_DIR)/client
SERVER_BIN_DIR = $(BIN_DIR)/server

.PHONY: all clean

client: $(CLIENT_OBJ)
	mkdir -p $(CLIENT_BIN_DIR)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o $(CLIENT_BIN_DIR)/$@

server: $(SERVER_OBJ)
	mkdir -p $(SERVER_BIN_DIR)
	$(CC) $(CFLAGS) $(SERVER_OBJ) -o $(SERVER_BIN_DIR)/$@

clean:
	rm -rf $(BIN_DIR) $(CLIENT_OBJ) $(SERVER_OBJ)