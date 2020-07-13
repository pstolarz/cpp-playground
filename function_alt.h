#ifndef __FUNCTION_ALT_H__
#define __FUNCTION_ALT_H__

namespace function_alt {

template<class Res, class ...Args>
struct _FunctorBase
{
    virtual Res operator() (Args... args) = 0;
    virtual ~_FunctorBase() = default;
};

template<class F, class Res, class ...Args>
struct _Functor: _FunctorBase<Res, Args...>
{
    _Functor(F& f): f(f) {}

    virtual Res operator() (Args... args)
    {
        return f(args...);
    }

private:
    F& f;
};

template<class T> struct Function;

template<class Res, class ...Args>
struct Function<Res(Args...)>
{
    Function() = default;

    ~Function() {
        if (_functor) delete _functor;
        _functor = nullptr; 
    }

    template<class F>
    Function& operator= (F& f) {
        _functor = new _Functor<F, Res, Args...>(f);
        return *this;
    }

    Res operator() (Args... args) {
        return _functor->operator()(args...);
    }

private:
    _FunctorBase<Res, Args...> *_functor = nullptr;
};

/*
 * test
 */
static int _f(long i, int& j)
{
    std::cout << "i: " << ++i << ", ";
    std::cout << "j: " << ++j << "\n";

    return (i+j);
}

struct _Fo {
    _Fo() = default;

    int operator() (short i, int& j)
    {
    std::cout << "i: " << --i << ", ";
    std::cout << "j: " << --j << "\n";

    return (i-j);
    }
};

void test(void)
{
    int i=0, j=0, res;
    Function<int(int, int&)> func;

    func = _f;
    res = func(i, j);
    std::cout << "res: " << res << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n";

    _Fo _fo;
    func = _fo;
    res = func(i, j);
    std::cout << "res: " << res << ", ";
    std::cout << "i: " << i << ", ";
    std::cout << "j: " << j << "\n";
}

} // namespace function_alt

#endif /* __FUNCTION_ALT_H__ */
