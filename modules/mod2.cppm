// module preamble (interface unit)
export module mod2;
import mod1;

namespace mod2 {

const char *g();

template<typename F>
const char *f1(F f) {
    return f();
}

export const char *f1_use1 = f1(mod1::f1);
export const char *f1_use2 = f1(g);

}

// implementation part (in one translation unit)
module :private;

namespace mod2 {

const char *g() {
    return "mod2::g()";
}

}
