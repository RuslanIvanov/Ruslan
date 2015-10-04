all: mybash

mybash: mybash.o
	g++ mybash.o -o mybash

mybash.o: mybash.cpp
	g++ -c mybash.cpp

clean:
	rm -rf *.o mybash
