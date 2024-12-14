CC=gcc

# optimisation 1 & 2
# CFLAGS=

# optimisation 3 & 4
# CFLAGS=-mavx -fopenmp

# optimisation 5 & 6
CFLAGS=-mavx512f -fopenmp

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
