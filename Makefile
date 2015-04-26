CXX = g++
CXXFLAGS = -Wall -Werror -std=c++0x -pedantic
OBJ = rshell.o ls.o

all: $(OBJ)
	@mkdir -p bin
	$(CXX) rshell.o -o bin/rshell
	$(CXX) ls.o -o bin/ls
	-@rm *.o 2>/dev/null || true

rshell.o: src/rshell.cpp
	$(CXX) $(CXXFLAGS) -c src/rshell.cpp

ls.o: src/ls.cpp
	$(CXX) $(CXXFLAGS) -c src/ls.cpp

clean:
	rm -r bin

