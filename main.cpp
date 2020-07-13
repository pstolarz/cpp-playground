#include <iostream>

#include "refs.h"
#include "function_alt.h"
#include "tuple_alt.h"
#include "templates_args.h"
#include "templates_spec.h"

int main(void)
{
    std::cout << "\n--------- refs::test()" << "\n";
    refs::test();

    std::cout << "\n--------- function_alt::test()" << "\n";
    function_alt::test();

    std::cout << "\n--------- tuple_alt::test()"  << "\n";
    tuple_alt::test();

    std::cout << "\n--------- templates_args::test()"  << "\n";
    templates_args::test();

    std::cout << "\n--------- templates_spec::test()"  << "\n";
    templates_spec::test();

    return 0;
}
