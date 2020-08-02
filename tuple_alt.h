#ifndef __TUPLE_ALT_H__
#define __TUPLE_ALT_H__

#include <utility>  // std::integer_sequence

namespace tuple_alt {

template<size_t index, class Head, class ...Tail>
struct _Tuple: _Tuple<index+1, Tail...>
{
    typedef typename std::remove_reference<Head>::type elem_type;
    Head&& h;

    typedef _Tuple<index+1, Tail...> base_type;

    constexpr _Tuple(Head&& h, Tail&&... tail):
        base_type(std::forward<decltype(tail)>(tail)...),
        h(std::forward<decltype(h)>(h)) {}

    Head&& get_elem() const {
        return h;
    }

    base_type& get_base() const {
        return static_cast<base_type&>(*this);
    }

    void *get_elem_addr(size_t n) const
    {
        return (n == index ?
            const_cast<void*>((const void*)&h) :
            base_type::get_elem_addr(n));
    }
};

template<size_t index, class Head>
struct _Tuple<index, Head>
{
    typedef typename std::remove_reference<Head>::type elem_type;
    Head&& h;

    constexpr _Tuple(Head&& h): h(std::forward<decltype(h)>(h)) {}

    Head&& get_elem() const {
        return h;
    }

    void *get_elem_addr(size_t n) const
    {
        return (n == index ?
            const_cast<void*>((const void*)&h) :
            nullptr);
    }

    static constexpr size_t size = index+1;
};

template<class ...List>
using Tuple = _Tuple<0, List...>;

template<class ...List>
Tuple<List...> constexpr tuple_alt(List&&... list)
{
    return Tuple<List...>(std::forward<decltype(list)>(list)...);
};


template<size_t index, class Tuple>
struct TupleTrait: TupleTrait<index-1, typename Tuple::base_type>
{};

template<class Tuple>
struct TupleTrait<0, Tuple>
{
    typedef typename std::remove_reference<Tuple>::type tuple_type;
    typedef typename Tuple::elem_type elem_type;
};

template<size_t n, class Tuple>
auto&& get_elem(Tuple& t)
{
    static_assert(n >= 0 && n < Tuple::size, "Invalid depth");
    return *reinterpret_cast<typename TupleTrait<n, Tuple>::elem_type*>(
        t.get_elem_addr(n));
}

template<class Seq>
struct _print_elems;

template<std::size_t ...Indexes>
struct _print_elems<std::integer_sequence<std::size_t, Indexes...>>
{
    template<class Tuple>
    static void print(Tuple& t) {
        auto l = {(std::cout << get_elem<Indexes>(t) << "\n", 0)...};
    }
};

template<size_t N, class Tuple>
void print_elems(Tuple& t) {
    _print_elems<std::make_index_sequence<N>>::print(t);
}

/*
 * test
 */
static void _f1(void) {}
static int _f2(int) { return 0; }

void test()
{
    int i = 3;
    double d = 4.5;
    char str[] = "lv-string";
    int *pi = &i;

    auto t = tuple_alt(1, 2.5, "rv-string", i, d, str, pi, _f1, _f2);
    print_elems<9>(t);
}

} // namespace tuple_alt

#endif /* __TUPLE_ALT_H__ */
