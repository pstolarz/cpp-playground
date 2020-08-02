#ifndef __MEM_PTR_H__
#define __MEM_PTR_H__

#include <type_traits>
#include <stdio.h>

namespace mem_ptr {

struct A { int a; };
struct B { int b; };
 
struct C : A, B {};

void test(void)
{
    printf("decltype(&C::a) == int C::* is %d\n",
        std::is_same<decltype(&C::a), int C::*>::value);
    printf("decltype(&C::a) == int C::A::* == int A::* is %d\n",
        std::is_same<decltype(&C::a), int C::A::*>::value &&
        std::is_same<decltype(&C::a), int A::*>::value);

    int C::*pmC = &C::a;
    printf("decltype(pmC) == int C::* is %d; pmC as &C::a value: %d\n",
        std::is_same<decltype(pmC), int C::*>::value, pmC);

    int A::*pmA = &C::A::a;
    printf("decltype(pmA) == int A::* is %d; pmA as &C::A::a value: %d\n",
        std::is_same<decltype(pmA), int A::*>::value, pmA);

    pmC = &C::b;
    printf("decltype(pmC) == int C::* is %d; pmC as &C::b value: %d\n",
        std::is_same<decltype(pmC), int C::*>::value, pmC);

    int B::*pmB = &C::B::b;
    printf("decltype(pmB) == int B::* is %d; pmB as &C::B::b value: %d\n",
        std::is_same<decltype(pmB), int B::*>::value, pmB);
}

} // namespace mem_ptr

#endif /* __MEM_PTR_H__ */
