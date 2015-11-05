#цель: зависимости
#[tab] команда

all: main monitor

main: main.o
	g++ main.o -o main  -lpthread

monitor: monitor.o
	g++ monitor.o -o monitor  -lpthread

main.o: main.cpp
	g++ -c main.cpp

monitor.o: monitor.cpp
	g++ -c monitor.cpp

clean:
	rm -rf *.o monitor
	rm -rf *.o main
