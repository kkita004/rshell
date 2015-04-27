CXX = g++
CXXFLAGS = -Wall -Werror -std=c++0x -pedantic
OBJ = rshell.o cp.o

all: $(OBJ)
	@mkdir -p bin
	$(CXX) rshell.o -o bin/rshell
	$(CXX) cp.o -o bin/cp
	-@rm *.o 2>/dev/null || true

rshell.o: src/rshell.cpp
	$(CXX) $(CXXFLAGS) -c src/rshell.cpp
cp.o: src/cp.cpp
	$(CXX) $(CXXFLAGS) -c src/cp.cpp
clean:
	rm -r bin

