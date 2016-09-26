CC = gcc

CFLAGS=-std=c99 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wno-missing-field-initializers -Warray-bounds -pedantic -fstrict-aliasing
LINKS=-lpcre
INCLUDES=-Isrc
SRC=src/blang.c src/core.c src/lexer.c src/constructs.c src/error.c
TPVSRC=src/tpv/slre.c

blang: clean
	cc -o blang $(SRC) $(TPVSRC) $(INCLUDES) $(CFLAGS) $(LINKS)
	@echo "end compilation--------->"

install:
	sudo cp blang /usr/local/bin

clean:
	rm -f blang
	rm -f build/*
