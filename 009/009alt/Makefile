all: main

main: main_dynamic.o
	g++ main_dynamic.o -o main_dynamic

main_dynamic.o: main_dynamic.cpp
	g++ -c main_dynamic.cpp

clean:
	rm -rf *.o main_dynamic
