.PHONY: all clean

CXX=gcc
CXXFLAGS+=-Wall -g

OBJ = main

all: $(OBJ)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lstdc++

%.disasm: %
	objdump --disassemble --disassembler-options=intel --source --line-numbers --demangle $< >$@

clean:
	$(RM) *.o $(OBJ)

