#ifndef __TEMPLATES_SPEC_H__
#define __TEMPLATES_SPEC_H__

namespace templates_spec {

template<class T1, class T2>
struct tm_1 {};

template<>
struct tm_1<int, int> {};

/*
// ERROR: number of template args != 3
template<>
struct tm_1<> {};

template<class T1>
struct tm_1<T1> {};

template<class T1, class T2>
struct tm_1<T1, int, T2> {};
*/


template<int I1, int I2>
struct tm_2 {
    constexpr static const char *name = "tm_2<I1, I2>";
};

template<int I1>
struct tm_2<I1, I1> {
    constexpr static const char *name = "tm_2<I1, I1>";
};

template<>
struct tm_2<3, 3> {
    constexpr static const char *name = "tm_2<3, 3>";
};


template<class T1, class ...Ts>
struct tm_3 {
    constexpr static const char *name = "tm_3<T1, ...>";
};

template<>
struct tm_3<int, int, int> {
    constexpr static const char *name = "tm_3<int, int, int>";
};

template<class T1>
struct tm_3<T1> {
    constexpr static const char *name = "tm_3<T1>";
};

// ERROR: tm_3 has at least 1 arg
/*
template<>
struct tm_3<> {};
*/


template<class ...Ts>
struct tm_4 {
    constexpr static const char *name = "tm_4<...>";
};

template<>
struct tm_4<> {
    constexpr static const char *name = "tm_4<>";
};


template<class T, class = T*>
struct tm_5 {
    constexpr static const char *name = "tm_5<T, = *T>";
};

template<class T>
struct tm_5<T, int> {
    constexpr static const char *name = "tm_5<T1, int>";
};

// ERROR: does not specialize any template arguments
/*
template<class T1, class T2>
struct tm_5<T1, T2> {};
*/

// ERROR: default template arguments may not be used in partial specializations
/*
template<class T = bool>
struct tm_5<int, T> {};
*/

template<int I, int N>
struct Loop: Loop<I+1, N> {
    void exe() {
        printf("%d\n", I);
        Loop<I+1, N>::exe();
    }
};

template<int N>
struct Loop<N, N> {
    void exe() {
        printf("%d\n", N);
    }
};

void test()
{
    printf("%s\n", tm_2<1, 2>::name);
    printf("%s\n", tm_2<1, 1>::name);
    printf("%s\n", tm_2<3, 3>::name);
    printf("\n");

    printf("%s\n", tm_3<int, float, void>::name);
    printf("%s\n", tm_3<int, int, int>::name);
    printf("%s\n", tm_3<int>::name);
    printf("\n");

    printf("%s\n", tm_4<int, int>::name);
    printf("%s\n", tm_4<>::name);
    printf("\n");

    printf("%s\n", tm_5<int>::name);
    printf("%s\n", tm_5<int, int>::name);
    printf("\n");

    Loop<5, 9> l;
    l.exe();
}

} // namespace templates_spec

#endif /* __TEMPLATES_SPEC_H__ */
