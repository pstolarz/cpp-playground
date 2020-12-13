.PHONY: all clean

CXX=gcc-10
CXXFLAGS+=-Wall -fcoroutines --std=c++2a

all: main

main: main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lstdc++

clean:
	$(RM) *.o main
