FLAGS = -lm -Wall -Wpedantic -Werror -std=c11

all: test

qoi.o: encoder.c
	gcc $< $(FLAGS) -s -o $@ 

debug: encoder.c
	gcc $< $(FLAGS) -g -o $@.o && gdb debug.o

test: qoi.o
	./run_test.sh
