#ifndef __REFS_H__
#define __REFS_H__

namespace refs {

template<typename T>
static void _f_ref(T& t) {
    std::cout << "ref_test(T&)\n";

    t++;
    std::cout << "  i:" << t << " [" << &t << "]\n";
}

template<typename T>
static void _f_ref(T&& t) {
    std::cout << ("ref_test(T&&)\n");

    t++;
    std::cout << "  i:" << t << " [" << &t << "]\n";
}

struct A {};

void test()
{
    /*
     * NOTES:
     * 1. rvalue initialization requires rvalue as an initializer. There is
     * not possible to initialize it with lvalue, only prvalue or xvalue are
     * allowed.
     *
     * 2. While initialized rvalue is treated as lvalue therefore to initialize
     * other rvalue of the same type with this rvalue there is a need to forward
     * it or cast as: rv2 = static_cast<decltype(rv1)>(rv1) which is equivalent
     * to forwarding.
     *
     * As a conclusion rvalue variable may be considered as lvalue (most cases)
     * or rvalue reference (depending on context), therefore proper usage of
     * variables of this type requires careful handling (e.g. type forwardnig).
     */
    int&& i = 0;    // prvalue

    i++;
    std::cout << "i:" << i << " [" << &i << "]\n";

    _f_ref(i);                              // calls T&
    _f_ref(std::forward<decltype(i)>(i));   // calls T&&

    int& j = i;     // i treated as lvalue

    _f_ref(j);                              // calls T&
    _f_ref(std::forward<decltype(j)>(j));   // calls T&

    A a;

    // ERROR: can't initialize rvalue with lvalue
    // A&& k = a;

    A&& k = std::move(a);   // OK: moved
    k = a;                  // OK: k treated as lvalue

    A&& l = A{};    // xvalue
    (void)l;        // get rid of compiler warning
}

} // namespace refs

#endif /* __REFS_H__ */
