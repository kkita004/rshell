CXX = g++
CXXFLAGS = -Wall -Werror
OBJ = rshell.o

all: $(OBJ)
	$(CXX) $(OBJ) -o rshell.out

rshell: /src/rshell.cpp
	$(CXX) $(CXXFLAGS) -c /src/rshell.cpp

clean:
	rm *.o *.out

