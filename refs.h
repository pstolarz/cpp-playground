#ifndef __REFS_H__
#define __REFS_H__

#include <type_traits>

namespace refs {

template<typename T>
static void T_ref(T& t) {
    std::cout << "T_ref(T&)\n";

    t++;
    std::cout << "  i:" << t << " [" << &t << "]\n";
}

template<typename T>
static void T_ref(T&& t) {
    std::cout << ("T_ref(T&&)\n");

    t++;
    std::cout << "  i:" << t << " [" << &t << "]\n";
}

static void int_ref(int& t) {
    std::cout << "int_ref(int&)\n";

    t++;
    std::cout << "  i:" << t << " [" << &t << "]\n";
}

static void int_ref(int&& t) {
    std::cout << ("int_ref(int&&)\n");

    t++;
    std::cout << "  i:" << t << " [" << &t << "]\n";
}

struct A {};

void test()
{
    /*
     * NOTES:
     * 1. rvalue-ref initialization requires rvalue as an initializer. There
     * is not possible to initialize it with lvalue, only prvalue or xvalue
     * are allowed.
     *
     * 2. While initialized rvalue-ref is treated as lvalue (possible to be
     * assigned to).
     *
     * 3. To initialize rvalue-ref with a rvalue-ref of the same type there is a
     * need to forward it or cast as: rv_ref2 = static_cast<decltype(rv1)>(rv_ref1)
     * (which is equivalent to forwarding).
     *
     * As a conclusion proper usage of rvalue-ref variables requires careful
     * handling via type forwardnig.
     */
    int&& i = 0;    // prvalue

    // ERROR: i decayed to int and can't be assigned to int&&
    // int&& ri = i;

    i++;
    std::cout << "i:" << i << " [" << &i << "]\n";

    T_ref(i);                               // calls T&
    T_ref(std::forward<decltype(i)>(i));    // calls T&&

    int_ref(i);                             // calls int&
    int_ref(std::forward<decltype(i)>(i));  // calls int&&

    int& ri = i;    // OK: i decayed to int

    T_ref(ri);                              // calls T&
    T_ref(std::forward<decltype(ri)>(ri));  // calls T&

    int_ref(ri);                                // calls int&
    int_ref(std::forward<decltype(ri)>(ri));    // calls int&

    T_ref(100);     // prvalue; calls T&&
    int_ref(100);   // prvalue; calls int&&

    A a;

    // ERROR: can't initialize rvalue-ref with lvalue
    // A&& ra1 = a;

    A&& ra1 = std::move(a); // OK: moved to A&&
    ra1 = a;                // OK: ra1 treated as lvalue (A&)

    A&& ra2 = A{};  // xvalue
    (void)ra2;      // get rid of compiler warning

    /*
     * universal reference
     */
    // OK: auto&& colapses to int& since i is decayed to int
    auto&& ri1 = i;
    static_assert(
        std::is_same<decltype(ri1), int&>::value, "decltype(ri1) != int&");

    // OK: int&& prvalue
    auto&& ri2 = 0;
    static_assert(
        std::is_same<decltype(ri2), int&&>::value, "decltype(ri2) != int&&");
}

} // namespace refs

#endif /* __REFS_H__ */
