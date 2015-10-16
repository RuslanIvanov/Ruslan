#цель: зависимости
#[tab] команда

all: client server

client: client.o
	g++ client.o -o client  -lpthread

server: server.o
	g++ server.o -o server  -lpthread

client.o: client.cpp
	g++ -c client.cpp

server.o: server.cpp
	g++ -c server.cpp

clean:
	rm -rf *.o server
	rm -rf *.o client
