#ifndef __ALIGN_C17_H__
#define __ALIGN_C17_H__

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <new>
#include <vector>

namespace align_c17 {

#if __cplusplus >= 201703L

static constexpr int alloc_retries = 100;

inline size_t get_algn(void *p)
{
    size_t algn, pi = reinterpret_cast<size_t>(p);
    for (algn = 1; !(pi & 1); pi >>= 1, algn <<= 1);
    return algn;
}

template<typename T>
void base_test(const char *t_name, const char *a_name,
    T* (*alloc_f)(void), void (*free_f)(T*))
{
    std::vector<T*> v{};
    size_t algn_min = (size_t)-1;
    void *ptr = nullptr;

    for (int i = 0; i < alloc_retries; i++) {
        T *p = alloc_f();
        size_t algn = get_algn(p);

        v.push_back(p);
        if (algn < algn_min) {
            algn_min = algn;
            ptr = reinterpret_cast<void*>(p);
        }
    }

    std::cout << a_name << "(" << t_name <<
        ") mem: " << ptr << ", algn: " << algn_min << "\n";

    for (auto p: v) free_f(p);
}

template<typename T>
inline void malloc_test(const char *t_name) {
    base_test<T>(t_name, "malloc",
        [] () -> T* { return (T*)malloc(sizeof(T)); },
        [] (T *p) { free(p); });
}

template<typename T>
inline void new_test(const char *t_name) {
    base_test<T>(t_name, "new",
        [] () -> T* { return new T{}; },
        [] (T *p) { delete p; });
}

struct alignas(256) s256_t {
    uint8_t tab[256];
};

#define ALGN_INFO(T) std::cout << "alignof(" #T "): " << alignof(T) << "\n"
#define MALLOC_TEST(T) malloc_test<T>(#T)
#define NEW_TEST(T) new_test<T>(#T)

void test()
{
    std::cout << "__STDCPP_DEFAULT_NEW_ALIGNMENT__: " <<
        __STDCPP_DEFAULT_NEW_ALIGNMENT__ << "\n";

    std::cout << "\n";

    ALGN_INFO(uint8_t);
    ALGN_INFO(uint16_t);
    ALGN_INFO(uint32_t);
    ALGN_INFO(uint64_t);
    ALGN_INFO(void*);
    ALGN_INFO(size_t);
    ALGN_INFO(std::max_align_t);
    ALGN_INFO(s256_t);

    std::cout << "\n";

    MALLOC_TEST(uint8_t);
    MALLOC_TEST(uint16_t);
    MALLOC_TEST(uint32_t);
    MALLOC_TEST(uint64_t);
    MALLOC_TEST(void*);
    MALLOC_TEST(size_t);
    MALLOC_TEST(std::max_align_t);
    MALLOC_TEST(s256_t);

    std::cout << "\n";

    NEW_TEST(uint8_t);
    NEW_TEST(uint16_t);
    NEW_TEST(uint32_t);
    NEW_TEST(uint64_t);
    NEW_TEST(void*);
    NEW_TEST(size_t);
    NEW_TEST(std::max_align_t);
    NEW_TEST(s256_t);
}

#else
void test() {
    std::cout << "Test not applicable\n";
}
#endif

} // namespace align_c17

#endif
