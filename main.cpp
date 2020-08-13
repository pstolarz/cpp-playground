#include <iostream>
#include <type_traits>

namespace {

// helper routine for printing pointer to member
template<typename CT, typename T, std::size_t N>
void _print_array(std::ostream& os, T (*arr)[N])
{
    for (std::size_t i = 0; i < N; i++) {
        os << static_cast<CT>((*arr)[i]);
        if (i+1 < N) os << ':';
    }
}

} // unnamed namespace

namespace std {

    // custom printing routine for pointer to object/function member
    template<typename F, typename B>
    ostream& operator<<(ostream& os, F B::*mp)
    {
        os << (is_member_object_pointer<decltype(mp)>::value ?
            "memb_obj_ptr [" : "memb_func_ptr [");

#if __cplusplus >= 201703L
        if constexpr
#else
        if
#endif
           (!(sizeof(F B::*) % sizeof(void*)))
        {
            // array of pointers
            _print_array<void*>(os,
                reinterpret_cast<void* (*)[sizeof(mp) / sizeof(void*)]>(&mp));
        } else {
            // unknown format
            _print_array<int>(os,
                reinterpret_cast<uint8_t (*)[sizeof(mp)]>(&mp));
        }
        return os << "]";
    }

} // std namespace

#include "refs.h"
#include "templates_args.h"
#include "templates_spec.h"
#include "member_ptr.h"
#include "check_type_expr.h"
#include "meta_list.h"
#include "function_alt.h"
#include "tuple_alt.h"

int main(void)
{
    std::cout << "\n--------- refs" << "\n";
    refs::test();

    std::cout << "\n--------- templates_args"  << "\n";
    templates_args::test();

    std::cout << "\n--------- templates_spec"  << "\n";
    templates_spec::test();

    std::cout << "\n--------- member_ptr"  << "\n";
    member_ptr::test();

    std::cout << "\n--------- check_type_expr"  << "\n";
    check_type_expr::test();

    std::cout << "\n--------- meta_list"  << "\n";
    meta_list::test();

    std::cout << "\n--------- function_alt" << "\n";
    function_alt::test();

    std::cout << "\n--------- tuple_alt"  << "\n";
    tuple_alt::test();

    return 0;
}
