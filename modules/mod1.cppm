// Global part (contains includes used by the module). Must occur first
// (even before module preamble).
module;
#include "enum.h"

// module preamble (interface unit)
export module mod1;

export namespace mod1 {

const char *f1();

const char *f2() {
    return "mod1::f2()";
}

_enum f3();

}
