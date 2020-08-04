#ifndef __TUPLE_ALT_H__
#define __TUPLE_ALT_H__

#include <type_traits>
#include <utility>  // C++14 std::integer_sequence

namespace tuple_alt {

template<std::size_t Index, typename Head, typename ...Tail>
struct _Tuple: _Tuple<Index+1, Tail...>
{
    Head&& h; // Head lvalue with possible references to Head rvalues

    _Tuple(Head&& h, Tail&&... tail):
        _Tuple<Index+1, Tail...>(std::forward<decltype(tail)>(tail)...),
        /*
         * NOTES:
         * 1. 'h(h)' doesn't work here since 'h' (as function argument) is
         * treated as Head lvalue and '_Tuple::h' is considered as Head rvalue,
         * therefore there is a need to forward 'h' or cast 'h' as:
         * 'h(static_cast<decltype(h)>(h))' which is equivalent to forwarding.
         *
         * 2. 'h(std::move(h))' also doesn't work here since 'std::move(h)' is
         * Head rvalue and '_Tuple::h' is considered in this case as Head lvalue.
         *
         * As a conclusion T&& may be considered as rvalue/lvalue reference
         * (depending on context) therefore any assignments between values of
         * such types requires explicit casting (type forwarding).
         */
        h(std::forward<decltype(h)>(h))
    {}

    Head&& get_elem() const {
        return std::forward<decltype(h)>(h);
    }
};

template<std::size_t Index, typename Head>
struct _Tuple<Index, Head>
{
    Head&& h;

    _Tuple(Head&& h):
        h(std::forward<decltype(h)>(h))
    {}

    Head&& get_elem() const {
        return std::forward<decltype(h)>(h);
    }

    static constexpr std::size_t size = Index+1;
};

template<typename ...List>
_Tuple<0, List...> tuple_alt(List&&... list)
{
    return _Tuple<0, List...>(std::forward<decltype(list)>(list)...);
};


template<std::size_t N, bool RecurCond, typename Tuple>
struct _get_tuple {};

template<std::size_t N, std::size_t Index, typename Head, typename ...Tail>
struct _get_tuple<N, false, _Tuple<Index, Head, Tail...>>:
    _get_tuple<N-1, !(N-1), _Tuple<Index+1, Tail...>>
{};

template<std::size_t Index, typename ...Types>
struct _get_tuple<0, true, _Tuple<Index, Types...>>
{
    using type = _Tuple<Index, Types...>;
};

template<std::size_t N, std::size_t Index, typename ...Types>
using get_tuple = _get_tuple<N, !N, _Tuple<Index, Types...>>;

template<std::size_t N, std::size_t Index, typename ...Types>
auto get_elem(const _Tuple<Index, Types...>& t)
    // C++11 required
    // -> decltype(std::declval<
    //    typename get_tuple<N, Index, Types...>::type>().get_elem())
{
    static_assert(N >= 0 && N < _Tuple<Index, Types...>::size, "Invalid depth");
    return static_cast<
        const typename get_tuple<N, Index, Types...>::type*>(&t)->get_elem();
}


template<typename Seq>
struct _print_elems;

template<std::size_t ...Indexes>
struct _print_elems<std::integer_sequence<std::size_t, Indexes...>>
{
    template<std::size_t Index, typename ...Types>
    static void print(const _Tuple<Index, Types...>& t) {
        // use initializer-list to ensure proper elements order
        auto l = {(std::cout << get_elem<Indexes>(t) << "\n", 0)...};
    }
};

template<std::size_t Index, typename ...Types>
void print_elems(const _Tuple<Index, Types...>& t) {
    _print_elems<std::make_index_sequence<sizeof...(Types)>>::print(t);
}


/*
 * test
 */
static void f() {}
struct A {};

std::ostream& operator<<(std::ostream& os, const A& a)
{
    std::cout << "struct A lvalue";
    return os;
}

std::ostream& operator<<(std::ostream& os, const A&& a)
{
    std::cout << "struct A rvalue";
    return os;
}

void test()
{
    int i = 3;
    double d = 4.5;
    char str[] = "lv-string";
    int *pi = &i;

    A a;
    auto t = tuple_alt(1, 2.5, "rv-string", i, d, str, pi, f, a, A{});
    print_elems(t);

    auto e = get_elem<2>(t);
    std::cout << "Tuple element #2: " << e << "\n";
}

} // namespace tuple_alt

#endif /* __TUPLE_ALT_H__ */
