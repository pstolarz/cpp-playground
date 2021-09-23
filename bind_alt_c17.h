#ifndef __BIND_ALT_C17__
#define __BIND_ALT_C17__

#include <cassert>
#include <functional>

namespace bind_alt_c17 {

#if __cplusplus >= 201703L

struct A { int a; };

int f(int i, A a1, A& a2) {
    std::cout << "f(): i:" << i << ", a1:" << a1.a++ << ", a2:" << a2.a++ << "\n";
    return 0;
}

void test()
{
    // similar to std::bind(f, 0, _1, _2)
    // no need to use cumbersome ref-wrappers
    auto b_f0 = [] (auto&& a1, auto&& a2) -> auto {
        return f(0, a1, a2);
    };

    A a1 = { .a = 0 };
    A a2 = { .a = 1 };
    b_f0(a1, a2);
    assert(a1.a == 0 && a2.a == 2);

    // may be used in similar context as bind in std::function
    std::function<int(A, A&)> func = b_f0;
    a1.a = 0;
    a2.a = 1;
    func(a1, a2);
    assert(a1.a == 0 && a2.a == 2);

    // similar to previous but with parametrized functor and int argument
    auto b = [] (auto f, int i) -> auto {
        return [f, i] (auto&& a1, auto&& a2) -> auto {
            return f(i, a1, a2);
        };
    };

    auto b_f1 = b(f, 1);
    a1.a = 0;
    a2.a = 1;
    b_f1(a1, a2);
    assert(a1.a == 0 && a2.a == 2);
}

#else
void test() {
    std::cout << "Test not applicable\n";
}
#endif

} // namespace bind_alt_c17

#endif
