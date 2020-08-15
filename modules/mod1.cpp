// global part
module;
#include "enum.h"

// module preamble (implementation unit)
module mod1;

// Implementation unit can't export anything; all its names are visible
// by other translation units from the same module and only interface module
// may export required names.
namespace mod1 {

const char *f1() {
    return "mod1::f1()";
}

_enum f3() {
    return _enum::ENUM_2;
}

}
