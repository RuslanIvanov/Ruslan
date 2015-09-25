
all: hello

hello: hello.o libprint.so
	gcc  hello.o -ldl -o hello

hello.o: hello.c
	gcc -fPIC -c hello.c

libprint.so: print.c
	gcc -fPIC -shared -o libprint.so print.c

clean:
	rm -rf *.o hello
	rm -rf *.so libprint.so
