#include <iostream>

/*
 * As part of class C virtual destructor ~C() C++ compiler creates 2 virtual
 * methods:
 *
 * 1. Main destructor method containing body of the compiled destructor
 *   (denoted later as ~C()<main>).
 *
 * 2. Deleting destructor method called for delete C* expression. Body of
 *    this methods calls the main destructor body followed by call to delete
 *    operator (denoted later as ~C()<delete>).
 */

#if __cplusplus >= 201103L
# define NOEXCEPT noexcept
#else
# define NOEXCEPT throw()
#endif

inline void *operator new(size_t sz) {
    std::cout << "operator new(" << sz << ")\n";
    return malloc(sz);
}

inline void operator delete(void *ptr) NOEXCEPT {
    std::cout << "operator delete(" << ptr << ")\n";
    free(ptr);
}

#if __cpp_sized_deallocation
inline void operator delete(void* ptr, size_t sz) NOEXCEPT {
    std::cout << "operator delete(" << ptr << ", " << sz << ")\n";
    free(ptr);
}
#endif

struct A {
    int a = 0;

    virtual ~A() { std::cout << "~A()\n"; }
};

struct A2 {
    int a2 = 0;

    virtual ~A2() { std::cout << "~A2()\n"; }
};

struct B: A2, A {
    int b = 1;

    virtual ~B() { std::cout << "~B()\n"; }
};

int main()
{
    {
        B b = B{};
        A *pa = &b;
        pa->~A();       // virtually calls ~B()<main>
        pa->A::~A();    // directly calls ~A()<main>
    }                   // directly calls ~B <main>

    std::cout << "\n";

    {
        B *b = new B{};
        A *a = b;
        std::cout << "a: " << a << ", b: " << b << "\n";
        delete a;       // virtually calls ~B<delete>
    }

    return 0;
}
