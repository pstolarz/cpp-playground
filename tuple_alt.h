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

/*
 * get_tuple helpers (need to be defined here since the type function
 * get_tuple is references by _Tuple::get_elem<N> member template).
 */
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

// final base of all tuples in the inheritance hierarchy (empty class)
using TupleEnd = _Tuple<_TypesList<>>;

// get tuple with given index from the tuple class
template<std::size_t N, typename Tuple>
using get_tuple = _get_tuple<N, !N, Tuple>;

namespace {

template<typename Head, typename ...Tail>
struct _Tuple<_TypesList<Head, Tail...>>: _Tuple<_TypesList<Tail...>>
{
    using type = _Tuple<_TypesList<Head, Tail...>>;
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

    // get element with a given index N (0 based)
    template<std::size_t N>
    auto get_elem() const
#if __cplusplus == 201103L
        // C++11 required
        -> decltype(
            std::declval<typename get_tuple<N, type>::type>().get_elem())
#endif
    {
        static_assert(N >= 0 && N <= sizeof...(Tail), "Invalid depth");
        return static_cast<
            const typename get_tuple<N, type>::type*>(this)->get_elem();
    }

#if __cplusplus >= 201402L
    void print_elems() const {
        _print_elems(
            static_cast<std::make_index_sequence<sizeof...(Tail)+1>*>(nullptr));
    }
#else
    void print_elems() const {
        _print_elems<0>(this);
    }
#endif

private:
    Head&& h; // Head lvalue with possible references to Head rvalues

    // C++ standard specific print_elems() helpers
#if __cplusplus >= 201402L
    /*
     * Print tuple elements via expanding integer indexes passed by
     * integer_sequence type (C++14).
     */
    template<std::size_t ...Indexes>
    void _print_elems(std::integer_sequence<std::size_t, Indexes...>*) const
    {
        // use initializer_list to ensure proper elements order
        auto l = {(std::cout << "#" << Indexes << ": " <<
            get_elem<Indexes>() << "\n", 0)...};

        // get rid of compiler warning
        (void)l;
    };
#else
    /*
     * Classical printing routine by iterating the tuple from most derived
     * class up to its teminator.
     */

    // final recursion (constexpr function returns 0 to make C++11 compiler happy)
    template<std::size_t I>
    static constexpr std::size_t _print_elems(const TupleEnd*) {
        return 0;
    }

    template<std::size_t I, typename ...Types>
    static void _print_elems(const _Tuple<_TypesList<Types...>> *t)
    {
        std::cout << "#" << I << ": " << t->get_elem() << "\n";
        _print_elems<I+1>(static_cast<
            const typename _Tuple<_TypesList<Types...>>::next*>(t));
    }
#endif
};

} // unnamed namespace

// create tuple
template<typename ...Types>
_Tuple<_TypesList<Types...>> make_tuple(Types&&... args)
{
    return _Tuple<_TypesList<Types...>>(std::forward<decltype(args)>(args)...);
};


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
    auto t = make_tuple(1, 2.5, "rv-string", i, d, str, pi, f, a, A{});
    t.print_elems();

    std::cout << "\n";

    std::cout << "#0: " << t.get_elem() << "\n";
    std::cout << "#2: " << t.get_elem<2>() << "\n";

    // ERROR - element out of size
    // auto e = t.get_elem<10>();
}

} // namespace tuple_alt

#endif /* __TUPLE_ALT_H__ */
