CC = gcc

CFLAGS=-std=c99 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wno-missing-field-initializers -Warray-bounds -pedantic -fstrict-aliasing
LINKS=-lpcre
INCLUDES=-Isrc
SRC=src/blang.c src/core.c src/lexer.c
STDSRC=src/std/constructs.c src/std/error.c src/std/io.c
TPVSRC=src/tpv/slre.c

blang: clean
	cc -o blang $(SRC) $(STDSRC) $(TPVSRC) $(INCLUDES) $(CFLAGS) $(LINKS)
	@echo "end compilation--------->"
	@echo "begin execution--------->"

install:
	sudo cp blang /usr/local/bin

clean:
	rm -f blang
	rm -f build/*
