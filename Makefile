CXX = g++
CXXFLAGS = -Wall -Werror -ansi -pedantic
OBJ = rshell.o

all: $(OBJ)
	mkdir -p bin
	$(CXX) $(OBJ) -o bin/rshell

rshell.o: src/rshell.cpp
	$(CXX) $(CXXFLAGS) -c src/rshell.cpp

clean:
	rm -r *.o bin

