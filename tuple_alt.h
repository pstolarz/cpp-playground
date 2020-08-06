#ifndef __TUPLE_ALT_H__
#define __TUPLE_ALT_H__

#include <type_traits>
#include <utility> // C++14 std::integer_sequence

namespace tuple_alt {
namespace {

template<typename ...Types>
struct _TypesList;

template<typename TypesList>
struct _Tuple;

// terminator
template<>
struct _Tuple<_TypesList<>> {};

template<typename Head, typename ...Tail>
struct _Tuple<_TypesList<Head, Tail...>>: _Tuple<_TypesList<Tail...>>
{
    using next = _Tuple<_TypesList<Tail...>>;

    _Tuple(Head&& h, Tail&&... tail):
        next(std::forward<decltype(tail)>(tail)...),
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

private:
    Head&& h; // Head lvalue with possible references to Head rvalues
};

} // unnamed namespace

// final base of all tuples in the inheritance hierarchy (empty class)
using TupleEnd = _Tuple<_TypesList<>>;

template<typename ...Types>
_Tuple<_TypesList<Types...>> tuple_alt(Types&&... args)
{
    return _Tuple<_TypesList<Types...>>(std::forward<decltype(args)>(args)...);
};


namespace {

template<std::size_t N, bool RecurCond, typename Tuple>
struct _get_tuple;

template<std::size_t N, typename ...Types>
struct _get_tuple<N, false, _Tuple<_TypesList<Types...>>>:
    _get_tuple<N-1, !(N-1), typename _Tuple<_TypesList<Types...>>::next>
{};

template<typename ...Types>
struct _get_tuple<0, true, _Tuple<_TypesList<Types...>>>
{
    using type = _Tuple<_TypesList<Types...>>;
};

} // unnamed namespace

template<std::size_t N, typename Tuple>
using get_tuple = _get_tuple<N, !N, Tuple>;

template<std::size_t N, typename ...Types>
auto get_elem(const _Tuple<_TypesList<Types...>>& t)
#if __cplusplus == 201103L
    // C++11 required
    -> decltype(std::declval<typename get_tuple<
        N, _Tuple<_TypesList<Types...>>>::type>().get_elem())
#endif
{
    static_assert(N >= 0 && N < sizeof...(Types), "Invalid depth");
    return static_cast<const typename get_tuple<
        N, _Tuple<_TypesList<Types...>>>::type*>(&t)->get_elem();
}


#if __cplusplus >= 201402L
namespace {

template<typename Seq>
struct _print_elems;

template<std::size_t ...Indexes>
struct _print_elems<std::integer_sequence<std::size_t, Indexes...>>
{
    template<typename ...Types>
    static void print(const _Tuple<_TypesList<Types...>>& t)
    {
        // use initializer-list to ensure proper elements order
        auto l = {(std::cout << "#" << Indexes << ": " <<
            get_elem<Indexes>(t) << "\n", 0)...};

        // get rid warning
        (void)l;
    }
};

} // unnamed namespace

/*
 * Print tuple elements via expanding integer indexes passed by
 * integer_sequence type (C++14).
 */
template<typename ...Types>
void print_elems(const _Tuple<_TypesList<Types...>>& t) {
    _print_elems<std::make_index_sequence<sizeof...(Types)>>::print(t);
}
#else
namespace {

/*
 * Final recursion step.
 * NOTE: the constexpr function returns (int)0 to make C++11 compiler happy.
 */
template<std::size_t I>
constexpr std::size_t _print_elems(const TupleEnd*) {
    return 0;
}

/*
 * Classical printing routine by iterating the tuple from most derived
 * class up to its first base class.
 */
template<std::size_t I, typename ...Types>
void _print_elems(const _Tuple<_TypesList<Types...>> *t)
{
    std::cout << "#" << I << ": " << t->get_elem() << "\n";
    _print_elems<I+1>(static_cast<
        const typename _Tuple<_TypesList<Types...>>::next*>(t));
}

} // unnamed namespace

template<typename ...Types>
inline void print_elems(const _Tuple<_TypesList<Types...>>& t)
{
    _print_elems<0>(&t);
}
#endif


/*
 * test
 */
static void f() {}
struct A {};

std::ostream& operator<<(std::ostream& os, const A& a)
{
    std::cout <<
        "struct A lvalue" << " addr: " << reinterpret_cast<const void*>(&a);
    return os;
}

std::ostream& operator<<(std::ostream& os, const A&& a)
{
    std::cout <<
        "struct A rvalue" << " addr: " << reinterpret_cast<const void*>(&a);
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

    std::cout << "\n";

    auto e = get_elem<2>(t);
    std::cout << "#2: " << e << "\n";
}

} // namespace tuple_alt

#endif /* __TUPLE_ALT_H__ */
