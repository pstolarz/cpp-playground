.PHONY: all clean

CXX=clang-11
CXXFLAGS+=-Wall --std=c++2a

# precompiled interface modules
PCMS=\
	mod1.pcm \
	mod2.pcm

OBJS=\
	mod1_i.o \
	mod1.o \
	mod2_i.o \
	main.o

all: main

mod2.pcm: mod1.pcm

clean:
	$(RM) *.o *.pcm main

main: $(PCMS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(addprefix -fmodule-file=,$(PCMS)) $(OBJS) -o $@ -lstdc++

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(addprefix -fmodule-file=,$(PCMS)) -c $< -o $@

%_i.o: %.pcm
	$(CXX) $(CXXFLAGS) $(addprefix -fmodule-file=,$(PCMS)) -c $< -o $@

%.pcm: %.cppm
	$(CXX) $(CXXFLAGS) $(addprefix -fmodule-file=,$(subst $<,,$+)) --precompile $< -o $@
