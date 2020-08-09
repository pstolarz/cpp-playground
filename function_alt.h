#include <functional>   // std::bad_function_call exception
#include <type_traits>

#ifndef __FUNCTION_ALT_H__
#define __FUNCTION_ALT_H__

namespace function_alt {

namespace {

// lightweight container used to pass member function along with its object
template<typename T, typename F, typename B>
struct _MembPtr
{
    T *t;
    F B::*f;

    static_assert(std::is_base_of<B, T>::value,
        "Member function declared to be called on wrong object");

    _MembPtr(T *t, F B::*f): t(t), f(f) {}
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
        F& f;

        Functor(F& f): f(f) {};

        Res operator() (Args... args) const
            // exception spec. taken from passed functor
            noexcept(noexcept(f(args...)))
            override
        {
            return f(args...);
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
        F B::*f;
        T *t;

        Functor(const _MembPtr<T, F, B>& mp): f(mp.f), t(mp.t) {};

        Res operator() (Args... args) const
            noexcept(noexcept((std::declval<B>().*f)(args...)))
            override
        {
            return (t->*f)(args...);
        }
    };

    FunctorBase *_functor = nullptr;

public:
    Function() = default;

    ~Function()
    {
        delete _functor;
        _functor = nullptr; 
    }

    // class with operator()
    template<typename F>
    Function& operator= (F& f)
    {
        delete _functor;
        _functor = new Functor<typename std::decay<F>::type>(f);
        return *this;
    }

    // regular function
    // passed types may differ than the declared types if they are convertible
    template<typename FRes, typename ...FArgs>
    Function& operator= (FRes(*f)(FArgs... args))
    {
        // TODO: check if passed types are convertible
        static_assert(sizeof...(FArgs) == sizeof...(Args),
            "Calling arguments don't match");

        delete _functor;
        _functor = new Functor<decltype(f)>(f);
        return *this;
    }

    // member function call
    template<typename T, typename F, typename B>
    Function& operator= (const _MembPtr<T, F, B>& mp)
    {
        delete _functor;
        _functor = new Functor<_MembPtr<T, F, B>>(mp);
        return *this;
    }

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

template<typename T, typename F, typename B>
_MembPtr<T, F, B> memb_ptr(T& t, F B::*f)
{
    return _MembPtr<T, F, B>(&t, f);
}


/*
 * test
 */
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

// may throw
static int f(long i, int& j) noexcept(false)
{
    std::cout << "f(long, int&) -> ";
    std::cout << "--i: " << --i << ", ";
    std::cout << "--j: " << --j << "\n";

    return i+j;
}

struct S2
{
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

void test(void)
{
    int i=0, j=0, sum;
    Function<int(int, int&)> func;

    S1 s1;
    func = s1;
    sum = func(i, j);
    std::cout << "sum: " << sum << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n\n";

    func = f;
    sum = func(i, j);
    std::cout << "sum: " << sum << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n\n";

    S2 s2;
    func = memb_ptr(s2, &S2::f);
    sum = func(i, j);
    std::cout << "sum: " << sum << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n\n";

    S3 s3;
    func = memb_ptr(&s3, &S3::f);
    sum = func(i, j);
    std::cout << "sum: " << sum << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n\n";

    func = memb_ptr(s3, &S2::f);
    sum = func(i, j);
    std::cout << "sum: " << sum << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n";

    // ERROR: S1 is not derived from S2
    // func = memb_ptr(s1, &S2::f);
}

} // namespace function_alt

#endif /* __FUNCTION_ALT_H__ */
