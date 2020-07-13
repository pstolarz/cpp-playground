CC=gcc

all: main

HEADERS = \
    refs.h \
    tuple_alt.h \
    function_alt.h \
    templates_args.h \
    templates_spec.h

main: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -lstdc++
clean:
	rm -f main
