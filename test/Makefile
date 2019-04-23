CCCC = g++

LIBS = -lpthread  -lrt \

CFLAGS=-c -Wall
LDFLAGS=

SOURCES=\
main.cpp\

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=exec

huwbuild: $(EXECUTABLE)
all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CCCC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CCCC) $(CFLAGS) $< -o $@  -MD $(LIBS)
	
#include $($(SOURCES).d)
include $(wildcard *.d)

install:
	cp $(EXECUTABLE)  ./$(EXECUTABLE)

uninstall:
	rm -f ./$(EXECUTABLE)
clean:
	rm -f *.o $(EXECUTABLE) *.d *~

