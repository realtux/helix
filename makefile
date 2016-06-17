CXX = gcc
CC = $(CXX)

blang: clean flex bison
	cc \
        -o blang \
		build/spec.l.c \
		build/spec.y.c

flex: bison
	flex \
        -o build/spec.l.c \
        parser/spec.l

bison:
	bison \
        -o build/spec.y.c \
        -d parser/spec.y

clean:
	rm -f blang
	rm -f build/*
