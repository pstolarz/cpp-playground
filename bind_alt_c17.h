#ifndef __BIND_ALT_C17__
#define __BIND_ALT_C17__

namespace bind_alt_c17 {

#include <cassert>

#if __cplusplus >= 201703L

struct A { int a; };

int f(int i, A a1, A& a2) {
    std::cout << "f(): i:" << i << ", a1:" << a1.a++ << ", a2:" << a2.a++ << "\n";
    return 0;
}

void test()
{
    auto bind_f = [] (auto i) {
        return [i] (auto&& a1, auto&& a2) -> auto {
            return f(i, a1, a2);
        };
    };

    auto bf_0 = bind_f(0.1);

    A a1 = { .a = 0 };
    A a2 = { .a = 1 };
    bf_0(a1, a2);
    assert(a1.a == 0 && a2.a == 2);
}

#else
void test() {
    std::cout << "Test not applicable\n";
}
#endif

} // namespace bind_alt_c17

#endif
