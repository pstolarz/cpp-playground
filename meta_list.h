#ifndef __META_LIST_H__
#define __META_LIST_H__

#include <type_traits>

namespace meta_list {

/* list type */
template<typename T, T Val, typename Next = void>
struct list
{
    static constexpr bool last = std::is_same<Next, void>::value;
    static constexpr T value = Val;
    using next = Next;
};


/* build list with reverse order of T-args */
template<typename Prev, typename T, T Val, T... Vals>
struct _build_rev_list:
    _build_rev_list<list<T, Val, Prev>, T, Vals...>
{};

template<typename Prev, typename T, T Val>
struct _build_rev_list<Prev, T, Val>
{
    using type = list<T, Val, Prev>;
};

template<typename T, T... Vals>
struct build_rev_list: _build_rev_list<void, T, Vals...>
{};


/* reverse order of list */
template<typename List, typename Prev = void>
struct reverse_list;

template<typename T, T Val, typename Next, typename Prev>
struct reverse_list<list<T, Val, Next>, Prev>:
    reverse_list<
        typename list<T, Val, Next>::next,
        list<T, list<T, Val, Next>::value, Prev>
    >
{};

template<typename Prev>
struct reverse_list<void, Prev>
{
    using type = Prev;
};


/* build list with given T-args */
template<typename T, T... Vals>
struct build_list:
    reverse_list<typename build_rev_list<T, Vals...>::type, void>
{};


/* print list */
template<typename T, T Val, typename Next = void>
void print_list(list<T, Val, Next>&&)
{
    std::cout << list<T, Val, Next>::value << std::endl;
    if constexpr (!list<T, Val, Next>::last)
        print_list(typename list<T, Val, Next>::next{});
}

template<typename T, T Val, typename Next = void>
void print_list(list<T, Val, Next>&)
{
    print_list(list<T, Val, Next>{});
}

void test()
{
    using list = build_list<int, 1, 2, 3>::type;
    print_list(list{});
}

} // namespace meta_list

#endif /* __META_LIST_H__ */
