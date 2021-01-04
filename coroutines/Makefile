.PHONY: all clean

CXX=gcc-10
CXXFLAGS+=-Wall -g -fcoroutines --std=c++2a

OBJS = \
       main \
       symmetric

all: $(OBJS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lstdc++

clean:
	$(RM) *.o $(OBJS)
