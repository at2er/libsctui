CC = gcc
CFLAGS = -g3 -std=c99 -pedantic -Wall -Wextra
AR = ar
PREFIX = /usr/local

HEADER_DIR = $(PREFIX)/include/sctui
TARGET_DIR = $(PREFIX)/lib

HEADER = sctui.h skb.h
TARGET = libsctui.a

SRC = sctui.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean debug install uninstall
all: $(TARGET) $(HEADER)
debug: all main.c
	$(CC) -o sctui main.c -g $(CFLAGS) -L. -lsctui

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(AR) -rcs $@ $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET) sctui

install: all
	mkdir -p $(HEADER_DIR) $(TARGET_DIR)
	cp -f $(HEADER) $(HEADER_DIR)
	cp -f $(TARGET) $(TARGET_DIR)/$(TARGET)

uninstall:
	rm -f $(HEADER_DIR)/$(HEADER) $(TARGET_DIR)/$(TARGET)
