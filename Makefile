CXX = g++
CXXFLAGS = -Wall -Werror -std=c++0x -pedantic
OBJ = rshell.o ls.o cp.o mv.o rm.o

all: $(OBJ)
	@mkdir -p bin
	$(CXX) rshell.o -o bin/rshell
	$(CXX) cp.o -o bin/cp
	$(CXX) ls.o -o bin/ls
	$(CXX) mv.o -o bin/mv
	$(CXX) rm.o -o bin/rm
	-@rm *.o 2>/dev/null || true

debugls: src/ls.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c -g src/ls.cpp
	$(CXX) ls.o -g -o bin/ls
	-@rm *.o 2>/dev/null || true

rshell.o: src/rshell.cpp
	$(CXX) $(CXXFLAGS) -c src/rshell.cpp

cp.o: src/cp.cpp
	$(CXX) $(CXXFLAGS) -c src/cp.cpp

ls.o: src/ls.cpp
	$(CXX) $(CXXFLAGS) -c src/ls.cpp

mv.o: src/mv.cpp
	$(CXX) $(CXXFLAGS) -c src/mv.cpp

rm.o: src/rm.cpp
	$(CXX) $(CXXFLAGS) -c src/rm.cpp

clean:
	rm -r bin

