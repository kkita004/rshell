CXX = g++
CXXFLAGS = -Wall -Werror -ansi -pedantic
OBJ = rshell.o

all: $(OBJ)
	$(CXX) $(OBJ) -o rshell.out

rshell.o: src/rshell.cpp
	$(CXX) $(CXXFLAGS) -c src/rshell.cpp

clean:
	rm *.o *.out

