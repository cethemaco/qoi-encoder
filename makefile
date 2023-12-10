FLAGS = -lm -Wall -Wpedantic -Werror -std=c11

all: test

qoi.o: main.c encoder.c
	gcc $^ $(FLAGS) -s -o $@ 

debug: main.c encoder.c
	gcc $^ $(FLAGS) -g -o $@.o && gdb debug.o

test: qoi.o
	./run_test.sh
