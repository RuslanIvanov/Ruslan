all: mykill

mykill: mykill.o
	g++ mykill.o -o mykill

mykill.o: mykill.cpp
	g++ -c mykill.cpp

clean:
	rm -rf *.o mykill
