CC = gcc
CLINKS = -mbmi2
CFLAGS = -Wall -Wextra -O3 -Iinclude -march=native -flto -std=c23

sources = $(wildcard src/*.c)
objects = $(patsubst src/%.c,build/%.o,$(sources))
executable = pike

.PHONY: all debug

all: $(executable)

debug: CFLAGS = -static -Wall -Wextra -g -Iinclude -std=c23
debug: clean $(executable)

$(executable): $(objects)
	$(CC) $(CFLAGS) $^ $(CLINKS) -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) $(CLINKS) -c $< -o $@

build:
	mkdir -p build

.PHONY: clean

clean:
	rm -f $(executable) $(objects)

