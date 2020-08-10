#ifndef __TUPLE_ALT_H__
#define __TUPLE_ALT_H__

#include <type_traits>
// C++14 std::integer_sequence, C++17 std::tuple_size, std::tuple_element
#include <utility>

namespace tuple_alt {

namespace detail {

template<typename ...Types>
struct _TypesList;

template<typename TypesList>
struct _Tuple;

// terminator
template<>
struct _Tuple<_TypesList<>> {};

/*
 * get_tuple helpers (need to be defined here since the type function
 * get_tuple is references by _Tuple::get<N> member template).
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

} // detail namespace

// final base of all tuples in the inheritance hierarchy (empty class)
using TupleEnd = detail::_Tuple<detail::_TypesList<>>;

// get tuple with given index from the tuple class
template<std::size_t N, typename Tuple>
using get_tuple = detail::_get_tuple<N, !N, Tuple>;

namespace detail {

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

    // same as get<0>()
    Head&& get() const {
        return std::forward<decltype(h)>(h);
    }

    // get element with a given index N (0 based)
    template<std::size_t N>
    auto get() const
        /*
         * It's required to use trailing return syntax here since w/o this
         * and >= C++14, 'auto' will return non-reference type therefore
         * causing returning by value (and calling copy/move constructors
         * for objects). 'decltype(auto)' would help in this case but it's
         * not supported for C++11.
         */
        -> decltype(std::declval<typename get_tuple<N, type>::type>().get())
    {
        static_assert(N >= 0 && N <= sizeof...(Tail), "Invalid depth");
        return static_cast<
            const typename get_tuple<N, type>::type*>(this)->get();
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
            get<Indexes>() << "\n", 0)...};

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
        std::cout << "#" << I << ": " << t->get() << "\n";
        _print_elems<I+1>(static_cast<
            const typename _Tuple<_TypesList<Types...>>::next*>(t));
    }
#endif
};

#if __cplusplus >= 201703L
template<std::size_t N, typename Tuple>
struct _tuple_element;

template<std::size_t N, typename... Types>
struct _tuple_element<N, _Tuple<_TypesList<Types...>>>
{
private:
    static_assert(N >= 0 && N < sizeof...(Types), "Invalid depth");
    using tuple_type = typename get_tuple<N, _Tuple<_TypesList<Types...>>>::type;

public:
    using type = decltype(std::declval<tuple_type>().get());
};
#endif

} // detail namespace

// create tuple
template<typename ...Types>
detail::_Tuple<detail::_TypesList<Types...>> make_tuple(Types&&... args)
{
    return detail::_Tuple<detail::_TypesList<Types...>>(
        std::forward<decltype(args)>(args)...);
};

} // tuple_alt namespace

#if __cplusplus >= 201703L
/*
 * std::tuple_size and std::tuple_element specialization (required for C++17
 * structured binding).
 *
 * NOTE: To not mess the std namespace by 'using function_alt::XYZ' full
 * symbols specification is provided here but main implementation of
 * tuple_element is provided by _tuple_element base class.
 */
namespace std {

template<typename... Types>
struct tuple_size<
        tuple_alt::detail::_Tuple<tuple_alt::detail::_TypesList<Types...>>>:
    integral_constant<size_t, sizeof...(Types)>
{};

template<size_t N, typename... Types>
struct tuple_element<
        N, tuple_alt::detail::_Tuple<tuple_alt::detail::_TypesList<Types...>>>:
    tuple_alt::detail::_tuple_element<
        N, tuple_alt::detail::_Tuple<tuple_alt::detail::_TypesList<Types...>>>
{};

} // std namespace
#endif


/*
 * test
 */
namespace tuple_alt {

static void f() {}
struct A {
    A() = default;

    // to make sure no extra A copies are performed
    A(const A&) = delete;
    A(A&&) = delete;
};

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
    char str[] = "RW-string";
    int *pi = &i;

    A a;
    auto t = make_tuple(
        1,              // 0
        2.5,            // 1
        "RO-string",    // 2
        i,              // 3
        d,              // 4
        str,            // 5
        pi,             // 6
        f,              // 7
        a,              // 8
        A{}             // 9
    );
    t.print_elems();

    std::cout << "\n";

    std::cout << "#0: " << t.get() << "\n";
    std::cout << "#2: " << t.get<2>() << "\n";

    // ERROR - element out of size
    // t.get<10>();

#if __cplusplus >= 201703L
    std::cout << "\n";

    // structured binding test
    auto& [e0, e1, e2, e3, e4, e5, e6, e7, e8, e9] = t;
    std::cout << "#0: " << e0 << "\n";
    std::cout << "#1: " << e1 << "\n";
    std::cout << "#2: " << e2 << "\n";
    std::cout << "#3: " << e3 << "\n";
    std::cout << "#4: " << e4 << "\n";
    std::cout << "#5: " << e5 << "\n";
    std::cout << "#6: " << e6 << "\n";
    std::cout << "#7: " << e7 << "\n";
    std::cout << "#8: " << e8 << "\n";
    std::cout << "#9: " << e9 << "\n";

    std::cout << "\n";

    // e3 is a reference to i
    e3++;
    std::cout << "#3: " << e3 << ", i: " << i << "\n";
#endif
}

} // namespace tuple_alt

#endif /* __TUPLE_ALT_H__ */
