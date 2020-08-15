#include <iostream>
#include "enum.h"

import mod1;
import mod2;

int main(void)
{
    std::cout << mod1::f1() << "\n";
    std::cout << mod1::f2() << "\n";
    std::cout << mod1::f3() <<  " == " << _enum::ENUM_2 << "\n";
    std::cout << mod2::f1_use1 << "\n";
    std::cout << mod2::f1_use2 << "\n";

    return 0;
}
