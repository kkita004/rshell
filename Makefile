CXX = g++
CXXFLAGS = -Wall -Werror -ansi -pedantic
OBJ = rshell.o

all: $(OBJ)
	$(CXX) $(OBJ) -o rshell.out

rshell.o:
	$(CXX) $(CXXFLAGS) -c src/rshell.cpp

clean:
	rm *.o *.out

