#include <iostream>

#include "refs.h"
#include "function_alt.h"
#include "tuple_alt.h"
#include "templates_args.h"
#include "templates_spec.h"
#include "check_type_expr.h"
#include "meta_list.h"
#include "member_ptr.h"

int main(void)
{
    std::cout << "\n--------- refs" << "\n";
    refs::test();

    std::cout << "\n--------- function_alt" << "\n";
    function_alt::test();

    std::cout << "\n--------- tuple_alt"  << "\n";
    tuple_alt::test();

    std::cout << "\n--------- templates_args"  << "\n";
    templates_args::test();

    std::cout << "\n--------- templates_spec"  << "\n";
    templates_spec::test();

    std::cout << "\n--------- check_type_expr"  << "\n";
    check_type_expr::test();

    std::cout << "\n--------- meta_list"  << "\n";
    meta_list::test();

    std::cout << "\n--------- member_ptr"  << "\n";
    member_ptr::test();

    return 0;
}
