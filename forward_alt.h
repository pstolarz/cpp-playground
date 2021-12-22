#ifndef __FORWARD_ALT_H__
#define __FORWARD_ALT_H__

#include <utility>
#include <type_traits>

namespace forward_alt {

// (1): called for lvalues and decayed rvalues
template<typename T>
constexpr inline T&& forward_alt(T& t) {
    return static_cast<T&&>(t);
}

// (2): called for prvalues and xvalues
template<typename T>
constexpr inline T&& forward_alt(T&& t) {
    return t;
}

// universal reference check
template<typename C, typename T>
constexpr void uref_check(T&& t) {
    static_assert(
        std::is_same<decltype(std::forward<T>(t)), C>::value &&
        std::is_same<decltype(forward_alt<T>(t)), C>::value,
        "uref_check() failed");
}

int test()
{
    int i;
    int& ri1 = i;
    int&& ri2 = 0;

    /*
     * check std::forward first
     */
    static_assert(std::is_same<
        decltype(std::forward<decltype(ri1)>(ri1)), int&>::value,
        "std::forward(): decltype(ri1) != int&");
    static_assert(std::is_same<
        decltype(std::forward<decltype(ri2)>(ri2)), int&&>::value,
        "std::forward(): decltype(ri2) != int&&");

    // ERROR: invalid forwarded type
    static_assert(!std::is_same<
        decltype(std::forward<int>(ri1)), int&>::value,
        "!(std::forward(): decltype(ri1) != int&)");
    static_assert(std::is_same<
        decltype(std::forward<int>(ri2)), int&&>::value,
        "std::forward(): decltype(ri2) != int&&");

    static_assert(std::is_same<
        decltype(std::forward<int>(0)), int&&>::value,
        "std::forward(): int-prvalue != int&&");
    static_assert(std::is_same<
        decltype(std::forward<int>(int{0})), int&&>::value,
        "std::forward(): int-xvalue != int&&");

    /*
     * check forward_alt
     */
    static_assert(std::is_same<
        decltype(forward_alt<decltype(ri1)>(ri1)), int&>::value,
        "forward_alt(): decltype(ri1) != int&");
    static_assert(std::is_same<
        decltype(forward_alt<decltype(ri2)>(ri2)), int&&>::value,
        "forward_alt(): decltype(ri2) != int&&");

    // ERROR: invalid forwarded type
    static_assert(!std::is_same<
        decltype(forward_alt<int>(ri1)), int&>::value,
        "!(forward_alt(): decltype(ri1) != int&)");
    static_assert(std::is_same<
        decltype(forward_alt<int>(ri2)), int&&>::value,
        "forward_alt(): decltype(ri2) != int&&");

    // calls (2); prvalue
    static_assert(std::is_same<
        decltype(forward_alt<int>(0)), int&&>::value,
        "forward_alt(): int-prvalue != int&&");
    // calls (2); xvalue
    static_assert(std::is_same<
        decltype(forward_alt<int>(int{0})), int&&>::value,
        "forward-alt(): int-xvalue != int&&");

    /*
     * universal reference checks
     */
    uref_check<int&>(i);
    uref_check<int&&>(0);

    std::cout << "All checks passed\n";

    return 0;
}
} // namespace forward_alt

#endif
