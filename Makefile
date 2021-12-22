.PHONY: all clean

CXX=gcc-10
CXXFLAGS+=-Wall --std=c++2a

HEADERS = \
    refs.h \
    forward_alt.h \
    templates_args.h \
    templates_spec.h \
    member_ptr.h \
    check_type_expr.h \
    meta_list.h \
    function_alt.h \
    tuple_alt.h \
    bind_alt_c17.h \
    align_c17.h

all: main

clean:
	rm -f main

main: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -o $@ -lstdc++
