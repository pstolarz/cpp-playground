#ifndef __TEMPLATES_ARGS_H__
#define __TEMPLATES_ARGS_H__

namespace templates_args {

template <typename> struct f_args;

template <typename R, typename... ArgTypes>
struct f_args<R(ArgTypes...)> {
    using type = R;
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

template <typename R, typename C, typename ...Args>
struct in_class<R (C::*)(Args...)>
{
    using type = R;
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

    std::cout << "f_t: int:" << std::is_same<f_args<f_t>::type, int>::value <<
        ", Args:" << f_args<f_t>::size << "\n";
    std::cout << "f_t: int(int, int&):" <<
        std::is_same<f_args<f_t>::type, int(int, int&)>::value <<
        ", Args:" << f_args<f_t>::size << "\n";

    std::cout << "\n";

    std::cout << "pf_t: int(*)(int, int&):" <<
        std::is_same<pf_t, int(*)(int, int&)>::value << "\n",
    std::cout << "pf_t(int, int&): int(*)(int, int&):" <<
        std::is_same<f_args<pf_t(int, int&)>::type, int(*)(int, int&)>::value <<
        ", Args:" << f_args<pf_t(int, int&)>::size << "\n";
    std::cout << "pf_t(): int(*)(int, int&):" <<
        std::is_same<f_args<pf_t()>::type, int(*)(int, int&)>::value <<
        ", Args:" << f_args<pf_t()>::size << "\n";

    std::cout << "\n";

    std::cout << "rf_t(double): int(&)(int, int&):" <<
        std::is_same<f_args<rf_t(double)>::type, int(&)(int, int&)>::value <<
        ", Args:" << f_args<rf_t(double)>::size << "\n";

    using pmf_t = decltype(&A::f);
    // using mf_t = std::remove_pointer<pmf_t>;
    // using rmf_t = mf_t&;

    std::cout << "\n";

    std::cout << "pmf_t(void*): int (A::*)(int, int&):" <<
        std::is_same<f_args<pmf_t(void*)>::type, int (A::*)(int, int&)>::value <<
        ", Args:" << f_args<pmf_t(void*)>::size << "\n";

    std::cout << "\n";

    std::cout << "in_class_m pmf_t: f_t:" <<
        std::is_same<in_class_m<pmf_t>::type, f_t>::value << ", class A:" <<
        std::is_same<in_class_m<pmf_t>::class_type, A>::value << "\n";

    std::cout << "in_class_f pmf_t: int:" <<
        std::is_same<in_class_f<pmf_t>::type, int>::value << ", class A:" <<
        std::is_same<in_class_f<pmf_t>::class_type, A>::value << "\n";

    std::cout << "\n";

    using pmm_t = decltype(&A::m);

    std::cout << "in_class_m pmm_t: int:" <<
        std::is_same<in_class_m<pmm_t>::type, int>::value << ", class A:" <<
        std::is_same<in_class_m<pmm_t>::class_type, A>::value << "\n";
}

} // namespace templates_args

#endif /* __TEMPLATES_ARGS_H__ */
