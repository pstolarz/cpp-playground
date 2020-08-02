CC=gcc-10

all: main

HEADERS = \
    refs.h \
    tuple_alt.h \
    function_alt.h \
    templates_args.h \
    templates_spec.h \
    check_type_expr.h \
    meta_list.h

main: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ --std=c++2a -lstdc++
clean:
	rm -f main
