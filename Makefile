CXX = g++
CXXFLAGS = -Wall -Werror
OBJ = rshell.o

all: $(OBJ)
	$(CXX) $(OBJ) -o rshell.out

rshell: rshell.cpp
	$(CXX) $(CXXFLAGS) -c rshell.cpp

clean:
	rm *.o *.out

