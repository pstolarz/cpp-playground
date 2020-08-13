#ifndef __MEM_PTR_H__
#define __MEM_PTR_H__

#include <type_traits>

namespace member_ptr {

struct A { int a; };
struct B { int b; };
 
struct C : A, B {};

void test(void)
{
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
}

} // namespace member_ptr

#endif /* __MEM_PTR_H__ */
