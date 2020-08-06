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
#if __cplusplus >= 201703L
template<typename T, T Val, typename Next>
void print_list(list<T, Val, Next>*)
{
    std::cout << list<T, Val, Next>::value << std::endl;
    // C++17 compile time check
    if constexpr (!list<T, Val, Next>::last)
        print_list(static_cast<typename list<T, Val, Next>::next*>(nullptr));
}
#else
// final recursion (constexpr function returns 0 to make C++11 compiler happy)
constexpr int _print_list(void*) {
    return 0;
}

template<typename T, T Val, typename Next>
void _print_list(list<T, Val, Next>*)
{
    std::cout << list<T, Val, Next>::value << std::endl;
    if (!list<T, Val, Next>::last)
        _print_list(static_cast<typename list<T, Val, Next>::next*>(nullptr));
}

template<typename T, T Val, typename Next>
inline void print_list(list<T, Val, Next> *p)
{
    _print_list(p);
}
#endif

#if __cplusplus >= 201402L
template<typename List>
static constexpr List *list_ptr = nullptr;
#else
template<typename List>
constexpr List* make_list_ptr() {
    return nullptr;
}
#endif

void test()
{
    using List = build_list<int, 1, 2, 3>::type;
#if __cplusplus >= 201402L
    print_list(list_ptr<List>);
#else
    print_list(make_list_ptr<List>());
#endif
}

} // namespace meta_list

#endif /* __META_LIST_H__ */
