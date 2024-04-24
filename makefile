CC = gcc

all : example

example :
	$(CC) -o test/example src/fmemt.c test/example.c

.PHONY : clean
clean:
	rm -f test/example
