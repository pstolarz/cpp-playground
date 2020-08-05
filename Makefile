.PHONY: all clean

CC=gcc-10

HEADERS = \
    refs.h \
    tuple_alt.h \
    function_alt.h \
    templates_args.h \
    templates_spec.h \
    check_type_expr.h \
    meta_list.h \
    mem_ptr.h

all: main

clean:
	rm -f main

main: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ --std=c++2a -lstdc++
