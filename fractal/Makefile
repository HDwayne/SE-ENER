CC=gcc
CFLAGS=-mavx -fopenmp
TARGET=fractal

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c algo.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
