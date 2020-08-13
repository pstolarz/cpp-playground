.PHONY: all clean

CC=gcc-10

HEADERS = \
    refs.h \
    templates_args.h \
    templates_spec.h \
    member_ptr.h \
    check_type_expr.h \
    meta_list.h \
    function_alt.h \
    tuple_alt.h

all: main

clean:
	rm -f main

main: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) -Wall $< -o $@ --std=c++2a -lstdc++
