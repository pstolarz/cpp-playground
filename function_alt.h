#ifndef __FUNCTION_ALT_H__
#define __FUNCTION_ALT_H__

#include <cstring>
#include <functional>   // std::bad_function_call exception
#include <type_traits>

/*
 * If configured - no dynamic memory allocation is performed.
 *
 * In this case it's not possible to pass rvalue (xvalue) object to Function,
 * since this would require moving the rvalue to a newly allocated object.
 * No allocation configuration requires the caller to maintain an object on
 * which the call in performed.
 */
#ifndef NO_DYNAMIC_ALLOCS
# define NO_DYNAMIC_ALLOCS  0
#endif

namespace function_alt {

namespace {

// lightweight container used to pass member function along with its object
template<typename T, typename F, typename B>
struct _MembPtr
{
    T *t = nullptr;
    F B::*f = nullptr;

    static_assert(std::is_base_of<B, T>::value,
        "Member function declared to be called on wrong object");

    _MembPtr() = delete;

    // _MembPtr object is unique
    _MembPtr(const _MembPtr&) = delete;
    _MembPtr& operator= (const _MembPtr&) = delete;

    _MembPtr(T *t, F B::*f): t(t), f(f) {}

    _MembPtr(T& t, F B::*f): t(&t), f(f) {}

#if NO_DYNAMIC_ALLOCS
    _MembPtr(_MembPtr&& mp) = default;
    _MembPtr& operator= (_MembPtr&& mp) = default;
#else
    bool alloced = false;

    _MembPtr(T&& t, F B::*f): f(f)
    {
        // move rvalue to a new object
        this->t = new T(std::move(t));
        alloced = true;
    }

    _MembPtr(_MembPtr&& mp)
    {
        operator= (std::forward<decltype(mp)>(mp));
    }

    _MembPtr& operator= (_MembPtr&& mp)
    {
        t = mp.t;
        f = mp.f;
        alloced = mp.alloced;

        mp.t = nullptr;
        mp.f = nullptr;
        mp.alloced = false;

        return *this;
    }

    ~_MembPtr() {
        if (alloced) {
            delete t;
            alloced = false;
        }
        t = nullptr;
        f = nullptr;
    }
#endif
};

} // unnamed namespace

template<typename T>
struct Function;

template<typename Res, typename ...Args>
struct Function<Res(Args...)>
{
private:
    struct FunctorBase
    {
        virtual Res operator() (Args... args) const = 0;
        virtual ~FunctorBase() = default;
    };

    // class with operator()
    template<typename F>
    struct Functor: FunctorBase
    {
        F *f = nullptr;

        Functor(F *f): f(f) {};

        Functor(F& f): f(&f) {};

#if NO_DYNAMIC_ALLOCS
        Functor(F&& f): f(&f) {}
#else
        bool alloced = false;

        Functor(F&& f)
        {
            // move rvalue to a new object
            this->f = new F(std::move(f));
            alloced = true;
        };

        ~Functor() {
            if (alloced) {
                delete f;
                alloced = false;
            }
            f = nullptr;
        }
#endif
        Res operator() (Args... args) const
            // exception spec. taken from passed functor
            noexcept(noexcept((*f)(args...)))
            override
        {
            return (*f)(args...);
        }
    };

    // regular function
    template<typename FRes, typename ...FArgs>
    struct Functor<FRes(*)(FArgs...)>: FunctorBase
    {
        using F = FRes(*)(FArgs...);
        F f;

        Functor(F f): f(f) {};

        Res operator() (Args... args) const
            // exception spec. taken from passed functor
            noexcept(noexcept(f(args...)))
            override
        {
            return f(args...);
        }
    };

    // member function of B called on object of T
    template<typename T, typename F, typename B>
    struct Functor<_MembPtr<T, F, B>>: FunctorBase
    {
        _MembPtr<T, F, B> mp;

        Functor(_MembPtr<T, F, B>&& mp):
            mp(std::move(mp))
        {};

        Res operator() (Args... args) const
            noexcept(noexcept((std::declval<B>().*(mp.f))(args...)))
            override
        {
            return ((mp.t)->*(mp.f))(args...);
        }
    };

    // helper routines for object initialization (via constructor or operator=)
    template<typename _Functor, typename F>
    Function& init(F&& f)
    {
#if NO_DYNAMIC_ALLOCS
        static_assert(
            sizeof(_Functor) <= sizeof(_functor_sp), "Memory space discrepancy");

        if (_functor) _functor->~FunctorBase();
        _functor = new (_functor_sp) _Functor(std::forward<decltype(f)>(f));
#else
        delete _functor;
        _functor = new _Functor(std::forward<decltype(f)>(f));
#endif
        return *this;
    }

    template<typename _Functor, typename F>
    Function& init(F *f)
    {
#if NO_DYNAMIC_ALLOCS
        static_assert(
            sizeof(_Functor) <= sizeof(_functor_sp), "Memory space discrepancy");

        if (_functor) _functor->~FunctorBase();
        _functor = new (_functor_sp) _Functor(f);
#else
        delete _functor;
        _functor = new _Functor(f);
#endif
        return *this;
    }

#if NO_DYNAMIC_ALLOCS
    // trivial max() implementation calculated on compilation time
    // (std::max() is not constexpr for C++11)
    template<typename T, T arg1, T arg2, T... args>
    struct max: max<T, (arg1 >= arg2 ? arg1 : arg2), args...> {};

    template<typename T, T arg1, T arg2>
    struct max<T, arg1, arg2> {
        static constexpr T value = (arg1 >= arg2 ? arg1 : arg2);
    };

    // used for compilation time calculations
    struct _dummy { Res operator()(Args...); };

    uint8_t _functor_sp[
        max<std::size_t,
            sizeof(Functor<_dummy>),
            sizeof(Functor<Res(*)(Args...)>),
            sizeof(Functor<_MembPtr<_dummy, Res(Args...), _dummy>>)>::value
    ];
#endif

    FunctorBase *_functor = nullptr;

public:
    // require use operator= afterwards
    explicit Function() = default;

    // Function object is unique
    Function(const Function&) = delete;
    Function& operator= (const Function&) = delete;

    Function(Function&& f)
    {
        operator= (std::forward<decltype(f)>(f));
    }

    Function& operator= (Function&& f)
    {
        _functor = f._functor;
        f._functor = nullptr;
        return *this;
    }

    ~Function()
    {
#if NO_DYNAMIC_ALLOCS
        if (_functor) _functor->~FunctorBase();
#else
        delete _functor;
#endif
        _functor = nullptr; 
    }

    // class with operator()
    template<typename F, typename Fd = typename std::remove_reference<F>::type>
    Function(F&& f)
    {
        init<Functor<Fd>>(std::forward<decltype(f)>(f));
    }

    template<typename F, typename Fd = typename std::remove_reference<F>::type>
    Function(F *f)
    {
        init<Functor<Fd>>(f);
    }

    template<typename F, typename Fd = typename std::remove_reference<F>::type>
    Function& operator= (F&& f)
    {
        return init<Functor<Fd>>(std::forward<decltype(f)>(f));
    }

    template<typename F, typename Fd = typename std::remove_reference<F>::type>
    Function& operator= (F *f)
    {
        return init<Functor<Fd>>(f);
    }

    // regular function
    // passed types may differ than the declared types if they are convertible
    template<typename FRes, typename ...FArgs>
    Function(FRes(*f)(FArgs... args))
    {
        // TODO: check if passed types are convertible
        static_assert(
            sizeof...(FArgs) == sizeof...(Args), "Calling arguments don't match");

        init<Functor<decltype(f)>>(f);
    }

    template<typename FRes, typename ...FArgs>
    Function& operator= (FRes(*f)(FArgs... args))
    {
        // TODO: check if passed types are convertible
        static_assert(
            sizeof...(FArgs) == sizeof...(Args), "Calling arguments don't match");

        return init<Functor<decltype(f)>>(f);
    }

    // member function call
    template<typename T, typename F, typename B>
    Function(const _MembPtr<T, F, B>&& mp)
    {
        init<Functor<_MembPtr<T, F, B>>>(std::forward<decltype(mp)>(mp));
    }

    template<typename T, typename F, typename B>
    Function& operator= (const _MembPtr<T, F, B>&& mp)
    {
        return init<Functor<_MembPtr<T, F, B>>>(std::forward<decltype(mp)>(mp));
    }

    // call with args
    Res operator() (Args... args) const noexcept(false)
    {
        if (!_functor) throw std::bad_function_call();
        return _functor->operator()(args...);
    }
};

template<typename T, typename F, typename B>
_MembPtr<T, F, B> memb_ptr(T *t, F B::*f)
{
    return _MembPtr<T, F, B>(t, f);
}

template<typename T, typename F, typename B,
    typename Td = typename std::remove_reference<T>::type>
_MembPtr<Td, F, B> memb_ptr(T& t, F B::*f)
{
    return _MembPtr<Td, F, B>(t, f);
}

template<typename T, typename F, typename B,
    typename Td = typename std::remove_reference<T>::type>
_MembPtr<Td, F, B> memb_ptr(T&& t, F B::*f)
{
    return _MembPtr<Td, F, B>(std::forward<decltype(t)>(t), f);
}


/*
 * test
 */
// may throw
static int f(long i, int& j) noexcept(false)
{
    std::cout << "f(long, int&) -> ";
    std::cout << "--i: " << --i << ", ";
    std::cout << "--j: " << --j << "\n";

    return i+j;
}

struct S1
{
    int operator() (short i, int& j)
    {
        std::cout << "S1::operator()(short, int&) -> ";
        std::cout << "++i: " << ++i << ", ";
        std::cout << "++j: " << ++j << "\n";

        return i+j;
    }
};

struct S2
{
    // ERROR: move constr. is required if passing by rvalue
    // S2(S2&&) = delete;

    int f(char i, int& j)
    {
        std::cout << "S2::f(char, int&) -> ";
        std::cout << "++i: " << (int)++i << ", ";
        std::cout << "--j: " << --j << "\n";

        return i+j;
    }
};

struct S3: S1, S2
{
    // may throw
    int f(double i, int& j) noexcept(false)
    {
        std::cout << "S3::f(double, int&) -> ";
        std::cout << "--i: " << (int)--i << ", ";
        std::cout << "++j: " << ++j << "\n";

        return i+j;
    }
};

inline static void print_res(int sum, int i, int j)
{
    std::cout << "sum: " << sum << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n\n";
}

using callback_t = Function<int(void*)>;

int test_callback(callback_t&& cb, void *arg)
{
    return cb(arg);
}

void test(void)
{
    int i=0, j=0, sum;
    Function<int(int, int&)> func;

    S1 s1;
    func = s1;
    sum = func(i, j);
    print_res(sum, i, j);

    func = f;
    sum = func(i, j);
    print_res(sum, i, j);

#if NO_DYNAMIC_ALLOCS
    S2 s2;
    func = memb_ptr(&s2, &S2::f);
#else
    func = memb_ptr(S2(), &S2::f);
#endif
    sum = func(i, j);
    print_res(sum, i, j);

    S3 s3;
    func = memb_ptr(&s3, &S3::f);
    sum = func(i, j);
    print_res(sum, i, j);

    func = memb_ptr(s3, &S2::f);
    sum = func(i, j);
    print_res(sum, i, j);

    // ERROR: S1 is not derived from S2
    // func = memb_ptr(s1, &S2::f);

#if NO_DYNAMIC_ALLOCS
    auto lambda = [](int i, int& j) -> int
#else
    func = [](int i, int& j) -> int
#endif
    {
        std::cout << "lambda closure -> ";
        std::cout << "i: " << i << ", ";
        std::cout << "j: " << j << "\n";

        return i+j;
    };
#if NO_DYNAMIC_ALLOCS
    func = lambda;
#endif
    sum = func(i, j);
    print_res(sum, i, j);

    auto func_mov = func;
    // ERROR: func moved to func_mov (run time exception)
    // func(i, j);

    // Calls lambda once again. It's safe to call it again here even though
    // the lambda is passed by rvalue, sine it is moved and stored inside
    // Function object.
    func_mov(i, j);
    print_res(sum, i, j);

    int res = test_callback(
        [](void *arg) -> int {
            std::cout << static_cast<const char*>(arg) << "\n";
            return strlen(static_cast<const char*>(arg));
        },
        const_cast<char*>("Hello world!")
    );
    std::cout << "callback returned " << res << "\n";
}

} // namespace function_alt

#endif /* __FUNCTION_ALT_H__ */
