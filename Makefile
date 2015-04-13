CXX = g++
CXXFLAGS = -Wall -Werror
OBJ = main.o

all: $(OBJ)
	$(CXX) $(OBJ) -o rshell

main: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm *.o *.out

