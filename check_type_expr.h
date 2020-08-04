#ifndef __CHECK_TYPE_EXPR_H__
#define __CHECK_TYPE_EXPR_H__

namespace check_type_expr {

#define T_EXPR(T) T*

template<typename T, typename...> using T_alias = T;
template <typename...> using void_alias = T_alias<void>;
template <typename...> using int_alias = T_alias<int>;

/* chk_spec<T, (T2)>; T2 is assumed to be always void therefore set as default */
template <class T, class = void>
struct chk_spec {
    static constexpr bool value = false;
};

/* chk_spec<T, void> spec. */
template <class T>
struct chk_spec<T, void_alias<T_EXPR(T)>> {
    static constexpr bool value = true;
};

/* ERROR : conflinting with previoud spec.: chk_spec<T, void>
template <class T>
struct chk_spec<T, void_alias<void>> {
    static constexpr int value = true;
};
*/

template<bool V>
struct chk_result {
    static constexpr auto value = V;
};

template<typename T>
chk_result<false> _chk_sfinae(...);

template<typename T>
chk_result<true> _chk_sfinae(int_alias<T_EXPR(T)>);

template<typename T>
struct chk_sfinae
{
    /* check _chk_sfinae(int) first, fall-back to _chk_sfinae(...) in case of failure */
    static constexpr bool value = decltype(_chk_sfinae<T>(0))::value;
};

void test()
{
    printf("Class-template spec. check:\n");
    printf("  Check int*  : %d\n", chk_spec<int>::value);
    printf("  Check int&* : %d\n", chk_spec<int&>::value);

    printf("SFINAE check:\n");
    printf("  Check int*  : %d\n", chk_sfinae<int>::value);
    printf("  Check int&* : %d\n", chk_sfinae<int&>::value);
}

} // namespace check_type_expr

#endif /* __CHECK_TYPE_EXPR_H__ */
