.PHONY: all clean

CXX=gcc
CXXFLAGS+=-Wall -fcoroutines --std=c++2a

OBJS = \
       main \
       symmetric

all: $(OBJS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lstdc++

clean:
	$(RM) *.o $(OBJS)
