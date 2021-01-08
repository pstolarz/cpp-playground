#ifndef __MEM_PTR_H__
#define __MEM_PTR_H__

#include <type_traits>

namespace member_ptr {

struct A {
    int a;
    int fa() { return a; }
};

struct B {
    int b;
    int fb() { return b; }
};

struct C : A, B {};

/*
 * Pointer to class member usually consists or 1 or 2 parts:
 *
 * 1. For object member pointer is an offset of the member inside a class.
 * 2. For function member pointer it consists of 2 parts:
 *   - The first one is a function address.
 *   - The second one is an offset of the class defining the function inside
 *     a class used to access the function member.
 *
 * As for this example:
 * &C::b is a type of int B::*, which is of course convertible to int C::*
 * and in the latter case is (usually) represented by an offset of B::b inside
 * C class. The offset may be seen as a superposition of two pointers: offset
 * of B::b inside B and offset of B inside C class.
 */
void test(void)
{
    // object member test
    std::cout << "decltype(&C::a) == int C::* is " <<
        std::is_same<decltype(&C::a), int C::*>::value << "\n",
    std::cout << "decltype(&C::a) == int C::A::* == int A::* is " <<
        (std::is_same<decltype(&C::a), int C::A::*>::value &&
         std::is_same<decltype(&C::a), int A::*>::value) << "\n";

    int C::*pmC = &C::a;
    std::cout << "decltype(pmC) == int C::* is " <<
        std::is_same<decltype(pmC), int C::*>::value <<
        "; pmC as &C::a value: " << pmC << "\n";

    int A::*pmA = &C::A::a;
    std::cout << "decltype(pmA) == int A::* is "<<
        std::is_same<decltype(pmA), int A::*>::value <<
        "; pmA as &C::A::a value: " << pmA << "\n";

    pmC = &C::b;
    std::cout << "decltype(pmC) == int C::* is " <<
        std::is_same<decltype(pmC), int C::*>::value <<
        "; pmC as &C::b value: " << pmC << "\n";

    int B::*pmB = &C::B::b;
    std::cout << "decltype(pmB) == int B::* is " <<
        std::is_same<decltype(pmB), int B::*>::value <<
        "; pmB as &C::B::b value: " << pmB << "\n";

    // function member test
    std::cout << "\ndecltype(&C::fa) == int (C::*)() is " <<
        std::is_same<decltype(&C::fa), int (C::*)()>::value << "\n",
    std::cout << "decltype(&C::fa) == int (C::A::*)() == int (A::*)() is " <<
        (std::is_same<decltype(&C::fa), int (C::A::*)()>::value &&
         std::is_same<decltype(&C::fa), int (A::*)()>::value) << "\n";

    int (C::*pfmC)() = &C::fa;
    std::cout << "decltype(pfmC) == int (C::*)() is " <<
        std::is_same<decltype(pfmC), int (C::*)()>::value <<
        "; pfmC as &C::fa value: " << pfmC << "\n";

    int (A::*pfmA)() = &C::A::fa;
    std::cout << "decltype(pfmA) == int (A::*)() is "<<
        std::is_same<decltype(pfmA), int (A::*)()>::value <<
        "; pfmA as &C::A::fa value: " << pfmA << "\n";

    pfmC = &C::fb;
    std::cout << "decltype(pfmC) == int (C::*)() is " <<
        std::is_same<decltype(pfmC), int (C::*)()>::value <<
        "; pfmC as &C::fb value: " << pfmC << "\n";

    int (B::*pfmB)() = &C::B::fb;
    std::cout << "decltype(pfmB) == int (B::*)() is " <<
        std::is_same<decltype(pfmB), int (B::*)()>::value <<
        "; pfmB as &C::B::fb value: " << pfmB << "\n";
}

} // namespace member_ptr

#endif /* __MEM_PTR_H__ */
