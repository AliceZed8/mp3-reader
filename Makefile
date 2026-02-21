CC = gcc
CFLAGS = -O3

SOURCES = main.c
TARGET = mp3-reader


all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

