CC=gcc
CFLAGS=
TARGET=fractal

.PHONY: all clean

all: $(TARGET)

$(TARGET): fractal.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
	rm -f *.o
