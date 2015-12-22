all: test

test: main.o
	g++ main.o -o test  -lpthread

main.o: main.cpp
	g++ -c main.cpp

clean:
	rm -rf *.o test
