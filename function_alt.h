#include <cstring>
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
    bool alloced;

    static_assert(std::is_base_of<B, T>::value,
        "Member function declared to be called on wrong object");

    _MembPtr() = delete;

    _MembPtr(T *t, F B::*f, bool alloced = false):
        t(t), f(f), alloced(alloced)
    {}

    // _MembPtr is unique
    _MembPtr(const _MembPtr&) = delete;
    _MembPtr& operator= (const _MembPtr&) = delete;

    _MembPtr(_MembPtr&& mp):
        t(mp.t), f(mp.f), alloced(mp.alloced)
    {
        mp.t = nullptr;
        mp.f = nullptr;
        mp.alloced = false;
    }

    _MembPtr& operator= (_MembPtr&& mp)
    {
        t = mp.t;
        f = mp.f;
        alloced = mp.alloced;

        mp.t = nullptr;
        mp.f = nullptr;
        mp.alloced = false;
    }

    ~_MembPtr() {
        if (alloced) {
            delete t;
            alloced = false;
        }
        t = nullptr;
        f = nullptr;
    }
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
        bool alloced = false;

        Functor(F& f): f(&f) {};

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

    FunctorBase *_functor = nullptr;

public:
    // require use operator= afterwards
    explicit Function() = default;

    // Function is unique
    Function(const Function&) = delete;
    Function& operator= (const Function&) = delete;

    Function(Function&& f)
    {
        _functor = f._functor;
        f._functor = nullptr;
    }

    Function& operator= (Function&& f)
    {
        _functor = f._functor;
        f._functor = nullptr;
    }

    ~Function()
    {
        delete _functor;
        _functor = nullptr; 
    }

    // class with operator()
    template<typename F, typename Fd = typename std::decay<F>::type>
    Function(F&& f)
    {
        _functor = new Functor<Fd>(std::forward<decltype(f)>(f));
    }

    template<typename F, typename Fd = typename std::decay<F>::type>
    Function& operator= (F&& f)
    {
        delete _functor;
        _functor = new Functor<Fd>(std::forward<decltype(f)>(f));
        return *this;
    }

    // regular function
    // passed types may differ than the declared types if they are convertible
    template<typename FRes, typename ...FArgs>
    Function(FRes(*f)(FArgs... args))
    {
        // TODO: check if passed types are convertible
        static_assert(sizeof...(FArgs) == sizeof...(Args),
            "Calling arguments don't match");

        _functor = new Functor<decltype(f)>(f);
    }

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
    Function(const _MembPtr<T, F, B>& mp)
    {
        _functor = new Functor<_MembPtr<T, F, B>>(mp);
    }

    template<typename T, typename F, typename B>
    Function& operator= (const _MembPtr<T, F, B>& mp)
    {
        delete _functor;
        _functor = new Functor<_MembPtr<T, F, B>>(mp);
        return *this;
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
    typename Td = typename std::decay<T>::type>
_MembPtr<Td, F, B> memb_ptr(T& t, F B::*f)
{
    return _MembPtr<Td, F, B>(&t, f);
}

template<typename T, typename F, typename B,
    typename Td = typename std::decay<T>::type>
_MembPtr<Td, F, B> memb_ptr(T&& t, F B::*f)
{
    return _MembPtr<Td, F, B>(
        // move rvalue to a new object
        new Td(std::move(t)),
        f, true);
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

    func = memb_ptr(S2(), &S2::f);
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

    func = [](int i, int& j) -> int
    {
        std::cout << "lambda closure -> ";
        std::cout << "i: " << i << ", ";
        std::cout << "j: " << j << "\n";

        return i+j;
    };
    sum = func(i, j);
    print_res(sum, i, j);

    auto func_mov = func;
    // ERROR: func moved to func_mov (run time exception)
    // func(i, j);

    // Calls lambda once again. It's safe to call it again here even though
    // the lambda was passed by rvalue, sine it has been moved and is stored
    // inside Function object.
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
