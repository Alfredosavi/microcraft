CC       ?= gcc

CFLAGS   ?= -O2
CFLAGS   += -Wall -Wextra -Wpedantic

CPPFLAGS ?=
LDFLAGS  ?=
LDLIBS   ?=

TARGET   := server
SRC      := $(wildcard *.c)
OBJ      := $(patsubst %.c, build/%.o, $(SRC))

all: build $(TARGET)

build:
	mkdir -p build

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o build/$@ $^ $(LDFLAGS) $(LDLIBS)

run: all
	./build/$(TARGET)

build/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean build run
