
all: main monitor

main: main.o libsem.so
	g++ main.o -o main  -lpthread -ldl \
	-Llib -lsem -Wl,-rpath,lib 


monitor: monitor.o libsem.so
	g++ monitor.o -o monitor  -lpthread -ldl \
	-Llib -lsem -Wl,-rpath,lib

main.o: main.cpp
	g++ -c main.cpp

monitor.o: monitor.cpp
	g++ -c monitor.cpp

libsem.so: lib/libsem.cpp
	g++ -fPIC -shared -o lib/libsem.so lib/libsem.cpp


clean:
	rm -rf *.o monitor
	rm -rf *.o main
	rm -rf *.so lib/libsem.so
