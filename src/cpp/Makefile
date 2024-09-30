# Makefile for different socket-servers

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror
LDFLAGS := -lpthread

#TODO: Add other executables
EXECUTABLES = \
	simple-client \
	sequential-server \
	threaded-server

all: $(EXECUTABLES)

simple-client: simple-client.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

sequential-server: utils.cpp sequential-server.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

threaded-server: threaded-server.cpp utils.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean format
clean:
	-rm -f *.o $(EXECUTABLES)

format:
	clang-format -i *.cpp *.hpp
