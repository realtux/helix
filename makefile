CC = gcc

CFLAGS=-std=c99 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wno-missing-field-initializers -Warray-bounds -pedantic -fstrict-aliasing
INCLUDES=-Isrc
SRC=src/blang.c src/core.c src/lexer.c
STDSRC=src/std/error.c src/std/io.c

blang: clean
	cc -o blang $(SRC) $(STDSRC) $(INCLUDES) $(CFLAGS)
	@echo "end compilation--------->"

clean:
	rm -f blang
	rm -f build/*
