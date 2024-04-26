.PHONY: default

default: osh clean

clean:
	rm shell.o main.o

osh: shell.o main.o
	gcc shell.o main.o -o osh

shell.o:
	gcc -c shell.c

main.o:
	gcc -c main.c
