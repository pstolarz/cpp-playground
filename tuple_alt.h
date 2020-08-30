#ifndef __TUPLE_ALT_H__
#define __TUPLE_ALT_H__

// C++14 std::integer_sequence, C++17 std::tuple_size, std::tuple_element
#include <utility>
#include <type_traits>

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

    // if Head&& is prvalue -> store by const copy
    // if Head&& is xvalue -> store by copy
    // if Head&& is lvalue -> keep reference
    using elem_type =
        // let T = ref-removed Head
        typename std::conditional<
            std::is_rvalue_reference<Head>::value,
            // T&& stored as T (xvalue copy)
            typename std::remove_reference<Head>::type,
            typename std::conditional<
                std::is_lvalue_reference<Head>::value,
                // T& stored as T& (lvalue ref)
                Head,
                // T stored as T (copy for xvalue or const copy for prvalue)
                typename std::conditional<
                    std::is_class<Head>::value ||
                    std::is_union<Head>::value,
                    // xvalue
                    Head,
                    // prvalue
                    const Head
                >::type
            >::type
        >::type;

    _Tuple(Head&& h, Tail&&... tail):
        next(std::forward<decltype(tail)>(tail)...),
        // no need to forward since elem is copy or lvalue to h
        elem(h)
    {}

    _Tuple(const _Tuple&) = default;
    _Tuple& operator= (const _Tuple&) = default;

    _Tuple(_Tuple&&) = default;
    _Tuple& operator= (_Tuple&&) = default;

    // same as get<0>()
    elem_type get() const {
        return elem;
    }

    // get element with a given index N (0 based)
    template<std::size_t N>
    auto get() const
        /*
         * It's required to use trailing return syntax here since w/o this
         * and >= C++14, 'auto' means non-reference type therefore causing
         * returning by value (and calling copy/move constructors for stored
         * lvalues). 'decltype(auto)' would help in this case but it's not
         * supported by C++11.
         */
        -> typename get_tuple<N, type>::type::elem_type
    {
        static_assert(N >= 0 && N <= sizeof...(Tail), "Invalid depth");
        return static_cast<const typename get_tuple<N, type>::type*>(this)->get();
    }

#if __cplusplus >= 201402L
    void print(std::ostream& os) const {
        _print(os,
            static_cast<std::make_index_sequence<sizeof...(Tail)+1>*>(nullptr));
    }
#else
    void print(std::ostream& os) const {
        _print<0>(os, this);
    }
#endif

private:
    elem_type elem;

    // C++ standard specific print() helpers
#if __cplusplus >= 201402L
    /*
     * Print tuple elements via expanding integer indexes passed by
     * integer_sequence type (C++14).
     */
    template<std::size_t ...Indexes>
    void _print(
        std::ostream& os, std::integer_sequence<std::size_t, Indexes...>*) const
    {
        // use initializer_list to ensure proper elements order
        auto l = {(os << "#" << Indexes << ": " << get<Indexes>() << "\n", 0)...};

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
    static constexpr std::size_t _print(std::ostream& os, const TupleEnd*)
        noexcept
    {
        return 0;
    }

    template<std::size_t I, typename ...Types>
    static void _print(std::ostream& os, const _Tuple<_TypesList<Types...>> *t)
    {
        os << "#" << I << ": " << t->get() << "\n";
        _print<I+1>(os,
            static_cast<const typename _Tuple<_TypesList<Types...>>::next*>(t));
    }
#endif
};

template<typename... Types>
std::ostream& operator<<(std::ostream& os, const _Tuple<_TypesList<Types...>>& t)
{
    t.print(os);
    return os;
}

} // detail namespace

// create tuple
template<typename ...Types>
detail::_Tuple<detail::_TypesList<Types...>> make_tuple(Types&&... args)
{
    return detail::_Tuple<detail::_TypesList<Types...>>(
        std::forward<decltype(args)>(args)...);
};

} // tuple_alt namespace

namespace std {

#if __cplusplus == 201103L
// C++11 doesn't have overloaded operator<< for printing nullptr_t
ostream& operator<<(ostream &os, nullptr_t) {
    return os << "nullptr";
}
#endif

#if __cplusplus >= 201703L
/*
 * std::tuple_size and std::tuple_element specialization (required for C++17
 * structured binding).
 *
 * NOTE: To not mess the std namespace by 'using function_alt::XYZ' full
 * symbols specification is provided here.
 */
template<typename... Types>
struct tuple_size<
        tuple_alt::detail::_Tuple<tuple_alt::detail::_TypesList<Types...>>>:
    integral_constant<size_t, sizeof...(Types)>
{};

template<size_t N, typename... Types>
struct tuple_element<
    N, tuple_alt::detail::_Tuple<tuple_alt::detail::_TypesList<Types...>>>
{
    static_assert(N >= 0 && N < sizeof...(Types), "Invalid depth");

    using type = typename tuple_alt::get_tuple<N,
             tuple_alt::detail::_Tuple<tuple_alt::detail::_TypesList<Types...>>>
         ::type::elem_type;
};
#endif

} // std namespace


/*
 * test
 */
namespace tuple_alt {

// may throw
static void f() noexcept(false) {}

struct A
{
    int a = 0;
    int get() noexcept { return a; }

    A() = default;
    A(int a) noexcept: a(a) {}

    // required since A xvalue is stored by copy
    A(const A&) = default;
    A(A&&) noexcept { a = -1; }
};

A f_a() noexcept { return A(2); }
A& f_a(A& a) noexcept { return a; }
A&& f_a(A&& a) noexcept { return std::forward<decltype(a)>(a); }

std::ostream& operator<<(std::ostream& os, const A& a)
{
    return os << "struct A lvalue: " << a.a << " [" <<
        reinterpret_cast<const void*>(&a) << "]";
}

std::ostream& operator<<(std::ostream& os, const A&& a)
{
    return os << "struct A rvalue: " << a.a << " [" <<
        reinterpret_cast<const void*>(&a) << "]";
}

void test()
{
    A a;
    int i = 3;
    const double d = 4.5;
    char str[] = "string";
    int *pi = &i;
    int A::*mp = &A::a;
    int (A::*mfp)() = &A::get;

    auto t = make_tuple(
        1,                  // 0: prvalue -> stored by const copy
        2.5,                // 1: prvalue
        "const string",     // 2: const lvalue -> stored as is
        i,                  // 3: lvalue -> stored as is
        d,                  // 4: const lvalue
        str,                // 5: lvalue
        pi,                 // 6: lvalue
        f,                  // 7: const lvalue
        a,                  // 8: lvalue
        A(1),               // 9: xvalue -> stored by copy
        f_a(),              // 10: xvalue
        f_a(a),             // 11: lvalue
        f_a(std::move(a)),  // 12: xvalue
        &A::a,              // 13: prvalue
        &A::get,            // 14: prvalue
        mp,                 // 15: lvalue
        mfp,                // 16: lvalue
        nullptr,            // 17: prvalue
        (void*)-1,          // 18: prvalue
        !pi,                // 19: prvalue
        make_tuple(0, 1.2)  // 20: xvalue
    );
    std::cout << t;

    std::cout << "\n";

    std::cout << "#0: " << t.get() << "\n";
    std::cout << "#2: " << t.get<2>() << "\n";

    // ERROR - element out of size
    // t.get<21>();

#if __cplusplus >= 201703L
    std::cout << "\n";

    // structured binding test
    auto& [e0, e1, e2, e3, e4, e5, e6, e7,
        e8, e9, e10, e11, e12, e13, e14, e15,
        e16, e17, e18, e19, e20] = t;

    std::cout << "#0: " << e0 << ", type check: "
        << std::is_same<decltype(e0), const int>::value << "\n";
    std::cout << "#1: " << e1 << ", type check: "
        << std::is_same<decltype(e1), const double>::value << "\n";
    std::cout << "#2: " << e2 << ", type check: " <<
        std::is_same<decltype(e2), const char(&)[13]>::value << "\n";
    std::cout << "#3: " << e3 << ", type check: " <<
        std::is_same<decltype(e3), int&>::value << "\n";
    std::cout << "#4: " << e4 << ", type check: " <<
        std::is_same<decltype(e4), const double&>::value << "\n";
    std::cout << "#5: " << e5 << ", type check: " <<
        std::is_same<decltype(e5), char (&)[7]>::value << "\n";
    std::cout << "#6: [" << e6 << "], type check: " <<
        std::is_same<decltype(e6), int*&>::value << "\n";
    std::cout << "#7: [" << reinterpret_cast<void*>(&e7) << "], type check: " <<
        std::is_same<decltype(e7), void(&)(void)>::value << "\n";
    std::cout << "#8: " << e8 << ", type check: " <<
        std::is_same<decltype(e8), A&>::value << "\n";
    std::cout << "#9: " << e9 << ", type check: " <<
        std::is_same<decltype(e9), A>::value << "\n";
    std::cout << "#10: " << e10 << ", type check: " <<
        std::is_same<decltype(e10), A>::value << "\n";
    std::cout << "#11: " << e11 << ", type check: " <<
        std::is_same<decltype(e11), A&>::value << "\n";
    std::cout << "#12: " << e12 << ", type check: " <<
        std::is_same<decltype(e12), A>::value << "\n";
    std::cout << "#13: " << e13 << ", type check: " <<
        std::is_same<decltype(e13), int A::* const>::value << "\n";
    std::cout << "#14: " << e14 << ", type check: " <<
        std::is_same<decltype(e14), int (A::* const)() noexcept>::value << "\n";
    std::cout << "#15: " << e15 << ", type check: " <<
        std::is_same<decltype(e15), int A::*&>::value << "\n";
    std::cout << "#16: " << e16 << ", type check: " <<
        std::is_same<decltype(e16), int (A::*&)()>::value << "\n";
    std::cout << "#17: " << e17 << ", type check: " <<
        std::is_same<decltype(e17), const std::nullptr_t>::value << "\n";
    std::cout << "#18: [" << e18 << "], type check: " <<
        std::is_same<decltype(e18), void* const>::value << "\n";
    std::cout << "#19: " << e19 << ", type check: " <<
        std::is_same<decltype(e19), const bool>::value << "\n";
    std::cout << "#20: " << e20 << ", type check: " <<
        std::is_same<decltype(e20),
            detail::_Tuple<detail::_TypesList<int, double>>>::value << "\n";

    std::cout << "\n";

    // ERROR: const lvalue
    // e2[0] = 'x';

    // e3 is a reference to i
    e3++;
    std::cout << "#3: " << e3 << " [" << &e3 << "], i: " <<
        i << "[" << &i << "]\n";

    std::cout << "#5: " << e5 << " [" << &e5 << "], str: " <<
        str << " [" << reinterpret_cast<void*>(str) << "]\n";

    // e8 is a reference to a
    e8.a--;
    std::cout << "#8: " << e8.a << " [" << &e8 << "], a: " <<
        a.a << " [" << &a << "]\n";

    // e9 is a copy of passed A xvalue
    std::cout << "#9: " << e9.a << " [" << &e9 << "]\n";
#endif
}

} // namespace tuple_alt

#endif /* __TUPLE_ALT_H__ */
