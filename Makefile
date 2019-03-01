HEADERS = parser.h

default: main

main.o: main.c $(HEADERS)
	gcc -c main.c -o main.o

parser.o: parser.c parser.h
	cc -c main.c inc.h

main: main.c parser.h
	cc -o notebook main.c parser.c

clean:
	-rm -f main.o
	-rm -f main
