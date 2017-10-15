CC=gcc
CFLAGS=-std=c99 \
	-O3 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith -Wcast-align \
	-Wstrict-prototypes -Wno-missing-field-initializers -Warray-bounds \
	-pedantic -fstrict-aliasing -g
LINKS=-lpcre -lpthread
INCLUDES=-I. -I./src

LIB_OBJS =
LIB_OBJS += build/objs/constructs.o
LIB_OBJS += build/objs/core.o
LIB_OBJS += build/objs/debugging.o
LIB_OBJS += build/objs/error.o
LIB_OBJS += build/objs/functions.o
LIB_OBJS += build/objs/helix.o
LIB_OBJS += build/objs/lexer.o
LIB_OBJS += build/objs/regex.o
LIB_OBJS += build/objs/std.o
LIB_OBJS += build/objs/std/output.o
LIB_OBJS += build/objs/std/string.o
LIB_OBJS += build/objs/std/thread.o

EXEC=helix

all: $(EXEC)
	@echo "execution begin---->"

$(EXEC): $(LIB_OBJS)
	@$(CC) $(CFLAGS) -o $@ $(LIB_OBJS) $(INCLUDES) $(LINKS)

$(LIB_OBJS): build/objs/%.o: src/%.c
	@echo "cc: $<"
	@$(CC) $(CFLAGS) -c $< $(INCLUDES) $(LINKS) -o $@

clean:
	find . -type f -name '*.o' -delete

install:
	sudo cp helix /usr/local/bin
