#цель: зависимости
#[tab] команда

all: hello

hello: hello.o
	gcc hello.o -o hello

hello.o: hello.c
	gcc -c hello.c

clean:
	rm -rf *.o hello
