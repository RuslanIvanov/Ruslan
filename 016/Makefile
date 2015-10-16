#цель: зависимости
#[tab] команда

all: server

server: server.o
	g++ server.o -o server  -lpthread

server.o: server.cpp
	g++ -c server.cpp

clean:
	rm -rf *.o server
