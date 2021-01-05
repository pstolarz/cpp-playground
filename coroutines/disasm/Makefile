.PHONY: all clean

CXX=gcc-10
CXXFLAGS+=-Wall -g -fcoroutines --std=c++2a

OBJ = main

all: $(OBJ)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lstdc++

%.disasm: %
	objdump --disassemble --disassembler-options=intel --source --line-numbers --demangle $< >$@

clean:
	$(RM) *.o $(OBJ)
