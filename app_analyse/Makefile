# Makefile pour app_analyse

CC=gcc
CFLAGS=-Wall -g
TARGET=app

.PHONY: all clean test

all: $(TARGET)

$(TARGET): app.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f fractal.bmp
	rm -rf ./data

# Utilisation : make test NAME=<nom_du_test>
test: $(TARGET)
	chmod +x test.sh
	@if [ -n "$(NAME)" ]; then \
		./test.sh $(TARGET); \
		mv ./data ./data-$(NAME)-fractal; \
		mv ./data-$(NAME)-fractal ../result/; \
	else \
		printf "Specify NAME=<name> to save the test output.\n"; \
	fi
