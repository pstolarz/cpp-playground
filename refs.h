#ifndef __REFS_H__
#define __REFS_H__

namespace refs {

template<class T>
static void _f_ref(T& t) {
    printf("ref_test(T&)\n");

    t++;
    printf("  i:%d [%p]\n", t, &t);
}

template<class T>
static void _f_ref(T&& t) {
    printf("ref_test(T&&)\n");

    t++;
    printf("  i:%d [%p]\n", t, &t);
}

void test()
{
    int&& i = 0;

    i++;
    printf("i:%d [%p]\n", i, &i);

    _f_ref(i);
    _f_ref(std::forward<decltype(i)>(i));

    int& j = i;

    _f_ref(j);
    _f_ref(std::forward<decltype(j)>(j));

//    int&& k = j;  // Error
    int&& k = 100;
    k = j;          // OK
}

} // namespace refs

#endif /* __REFS_H__ */
