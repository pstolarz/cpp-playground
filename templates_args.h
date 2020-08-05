#ifndef __TEMPLATES_ARGS_H__
#define __TEMPLATES_ARGS_H__

namespace templates_args {

template <typename> struct f_args;

template <typename F, typename... ArgTypes>
struct f_args<F(ArgTypes...)> {
    using type = F;
    static const int size = static_cast<int>(sizeof...(ArgTypes));
};

int f(int, int&);
struct A {
    int m;
    int f(int, int&);
};

template <typename> struct in_class;

template <typename T, typename C>
struct in_class<T C::*>
{
    using type = T;
    using class_type = C;
};

template <typename T, typename C, typename ...Args>
struct in_class<T (C::*)(Args...)>
{
    using type = T;
    using class_type = C;
};

template<typename T>
using in_class_m = in_class<T>;

template<typename T>
using in_class_f = in_class<T>;

void test()
{
    using f_t = decltype(f);
    using pf_t = f_t*;
    using rf_t = f_t&;

    printf("f_t: int:%d, Args:%d\n",
        std::is_same<f_args<f_t>::type, int>::value,
        f_args<f_t>::size);
    printf("f_t: int(int, int&):%d, Args:%d\n",
        std::is_same<f_args<f_t>::type,
        int(int, int&)>::value, f_args<f_t>::size);

    printf("\n");

    printf("pf_t: int(*)(int, int&):%d\n",
        std::is_same<pf_t, int(*)(int, int&)>::value);
    printf("pf_t(int, int&): int(*)(int, int&):%d, Args:%d\n",
        std::is_same<f_args<pf_t(int, int&)>::type, int(*)(int, int&)>::value,
        f_args<pf_t(int, int&)>::size);
    printf("pf_t(): int(*)(int, int&):%d, Args:%d\n",
        std::is_same<f_args<pf_t()>::type, int(*)(int, int&)>::value,
        f_args<pf_t()>::size);

    printf("\n");

    printf("rf_t(double): int(&)(int, int&):%d, Args:%d\n",
        std::is_same<f_args<rf_t(double)>::type, int(&)(int, int&)>::value,
        f_args<rf_t(double)>::size);

    using pmf_t = decltype(&A::f);
    // using mf_t = std::remove_pointer<pmf_t>;
    // using rmf_t = mf_t&;

    printf("\n");

    printf("pmf_t(void*): int (A::*)(int, int&):%d, Args:%d\n",
        std::is_same<f_args<pmf_t(void*)>::type, int (A::*)(int, int&)>::value,
        f_args<pmf_t(void*)>::size);

    printf("\n");

    printf("in_class_m pmf_t: f_t:%d, class A:%d\n",
        std::is_same<in_class_m<pmf_t>::type, f_t>::value,
        std::is_same<in_class_m<pmf_t>::class_type, A>::value);
    
    printf("in_class_f pmf_t: int:%d, class A:%d\n",
        std::is_same<in_class_f<pmf_t>::type, int>::value,
        std::is_same<in_class_f<pmf_t>::class_type, A>::value);

    printf("\n");

    using pmm_t = decltype(&A::m);

    printf("in_class_m pmm_t: int:%d, class A:%d\n",
        std::is_same<in_class_m<pmm_t>::type, int>::value,
        std::is_same<in_class_m<pmm_t>::class_type, A>::value);
}

} // namespace templates_args

#endif /* __TEMPLATES_ARGS_H__ */
