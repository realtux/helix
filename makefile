CXX = gcc
CC = $(CXX)
CFLAGS=-std=c99 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wno-missing-field-initializers -Warray-bounds -pedantic -fstrict-aliasing

blang: clean
	cc -o blang src/blang.c src/lexer.c $(CFLAGS)
	@echo "end compilation--------->"

clean:
	rm -f blang
	rm -f build/*
