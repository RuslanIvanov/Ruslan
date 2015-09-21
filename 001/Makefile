#цель: зависимости
#[tab] команда

all: hello

hello: hello.o libprint.so
	gcc  hello.o -ldl -o hello

hello.o: hello.c
	gcc -fPIC -c hello.c

print.o: print.c
	gcc -fPIC -c print.c

libprint.so: print.o
	gcc -shared -o libprint.so print.o

clean:
	rm -rf *.o hello
	rm -rf *.so libprint.so
