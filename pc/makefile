CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

TARGET  := server
SRC     := $(wildcard *.c)
OBJ     := $(patsubst %.c, build/%.o, $(SRC))

all: build $(TARGET)

build:
	mkdir -p build

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o build/$@ $(LDFLAGS)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean build
