#include <coroutine>
#include <exception>
#include <iostream>
#include <optional>
#include <set>

namespace std {

/*
 * operator<< overloads for std::optional printing
 */
template<typename T>
ostream& operator<<(ostream& os, optional<T>& opt)
{
    if (opt.has_value()) {
        return os << opt.value();
    }
    return os << "<empty>";
}

template<typename T>
inline ostream& operator<<(ostream& os, optional<T>&& opt)
{
    // calls operator<<(ostream&, optional<T>&)
    return operator<<(os, opt);
}

} // std namespace


template<typename Promise>
struct Awaitable
{
    Awaitable(bool ready, bool suspend, int resume) noexcept:
        _ready(ready), _suspend(suspend), _resume(resume)
    {
        std::cout << "Awaitable::Awaitable(); " << (void*)this << "\n";
    }

    ~Awaitable() noexcept {
        std::cout << "Awaitable::~Awaitable(); " << (void*)this << "\n";
    }

    // not necessary since awaitable == awaiter for this class,
    // but added to provide execution point printout
    Awaitable operator co_await() noexcept {
        std::cout << "Awaitable::operator co_await()\n";
        return std::move(*this);
    }

    bool await_ready() const noexcept {
        std::cout << "Awaitable::await_ready() -> " << _ready << "\n";
        return _ready;
    }

    bool await_suspend(std::coroutine_handle<Promise> h) const noexcept {
        std::cout << "Awaitable::await_suspend() -> " << _suspend << "\n";
        return _suspend;
    }

    int await_resume() const noexcept {
        std::cout << "Awaitable::await_resume() -> " << _resume << "\n";
        return _resume;
    }

private:
    bool _ready;
    bool _suspend;
    int _resume;
};

template<typename Promise>
struct Yieldable
{
    Yieldable(int yield) noexcept:
        _yield(yield)
    {
        std::cout << "Yieldable::Yieldable(); " << (void*)this << "\n";
    }

    ~Yieldable() noexcept {
        std::cout << "Yieldable::~Yieldable(); " << (void*)this << "\n";
    }

    // always suspend
    bool await_ready() const noexcept {
        std::cout << "Yieldable::await_ready() -> 0\n";
        return false;
    }

    // always suspend
    bool await_suspend(std::coroutine_handle<Promise> h) const noexcept {
        std::cout << "Yieldable::await_suspend() -> 1\n";
        return true;
    }

    int await_resume() const noexcept {
        std::cout << "Yieldable::await_resume() -> " << _yield << "\n";
        return _yield;
    }
private:
    int _yield;
};

template<typename InitialSuspend, typename FinalSuspend>
struct Result
{
    struct Promise
    {
        Promise() noexcept:
            _ready(false), _suspend(true), _results()
        {}

        Promise(bool ready, bool suspend) noexcept:
            _ready(ready), _suspend(suspend), _results()
        {
            std::cout << "Promise::Promise(); " << (void*)this << "\n";
        }

        ~Promise() noexcept {
            std::cout << "Promise::~Promise(); " << (void*)this << "\n";

            // inform associated objects about their promise removal
            for (auto r: _results) { r->_promise = nullptr; }
            _results.clear();
        }

        Awaitable<Promise> await_transform(int resume) const noexcept {
            std::cout << "Promise::await_transform()\n";
            return {_ready, _suspend, resume};
        }

        Yieldable<Promise> yield_value(int yield) const noexcept {
            std::cout << "Promise::yield_value()\n";

            // inform associated objects about the yield code
            for (auto r: _results) { r->_yield_code = yield; }
            return {yield};
        }

        Result get_return_object() noexcept {
            std::cout << "Promise::get_return_object()\n";
            return this;
        }

        InitialSuspend initial_suspend() const noexcept {
            std::cout << "Promise::initial_suspend()\n";
            return {};
        }

        FinalSuspend final_suspend() const noexcept {
            std::cout << "Promise::final_suspend()\n";
            return {};
        }

        void return_void() const noexcept {
            std::cout << "Promise::return_void()\n";
        }

        void return_value(int ret_code) noexcept {
            std::cout << "Promise::return_value()\n";

            // inform associated objects about the return code
            for (auto r: _results) { r->_ret_code = ret_code; }
        }

        void unhandled_exception() noexcept {
            std::cout << "Promise::unhandled_exception()\n";

            // inform associated objects about the raised exception
            for (auto r: _results) { r->_except = std::current_exception(); }
        }

    private:
        bool _ready;
        bool _suspend;
        std::set<Result*> _results;

        friend struct Result;
    };

    using promise_type = Promise;
    using Handle = std::coroutine_handle<Promise>;

    Result(Promise *promise) noexcept:
        _promise(promise), _ret_code(), _except(nullptr), _yield_code(0)
    {
        std::cout << "Result::Result(); " << (void*)this << "\n";

         // bind the object with its promise
        _promise->_results.insert(this);
    }

    ~Result() {
        std::cout << "Result::~Result(); " << (void*)this << "\n";

        // unbind the object from its promise
        if (_promise) _promise->_results.erase(this);
    }

    auto ret_code() {
        return _ret_code;
    }

    auto yield_code() {
        return _yield_code;
    }

    auto exception() {
        return _except;
    }

    bool done() {
        return (_promise ? Handle::from_promise(*_promise).done() : true);
    }

    void resume() {
        if (_promise)
            Handle::from_promise(*_promise).resume();
    }

    void destroy() {
        if (_promise)
            Handle::from_promise(*_promise).destroy();
    }

private:
    // if null promise has been freed
    Promise *_promise;

    std::optional<int> _ret_code;
    std::exception_ptr _except;
    int _yield_code;

};


constexpr const bool AWAIT_READY       = true;
constexpr const bool AWAIT_NOT_READY   = false;
constexpr const bool AWAIT_SUSPEND     = true;
constexpr const bool AWAIT_NOT_SUSPEND = false;

using ResultAA = Result<std::suspend_always, std::suspend_always>;
using ResultAN = Result<std::suspend_always, std::suspend_never>;
using ResultNN = Result<std::suspend_never, std::suspend_never>;
using ResultNA = Result<std::suspend_never, std::suspend_always>;

ResultAA co_return_1(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND) {
    co_return 1;
}

ResultNN co_return_2(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND) {
    co_return 2;
}

ResultNA co_await_1(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND) {
    int ares = co_await 1;
    std::cout << "co_await -> " << ares << "\n";
    // co_return;
}

ResultAN co_await_2(bool = AWAIT_READY, bool = AWAIT_SUSPEND) {
    int ares = co_await 2;
    std::cout << "co_await -> " << ares << "\n";
    co_return ares;
}

ResultNN co_await_3(bool = AWAIT_NOT_READY, bool = AWAIT_NOT_SUSPEND) {
    int ares = co_await 3;
    std::cout << "co_await -> " << ares << "\n";
    co_return ares;
}

ResultNA co_yield_1()
{
    int i=0, yres;

    do {
        yres = co_yield i++;
        std::cout << "co_yield -> " << yres << "\n";
    } while (yres < 5);

    co_return 1;
}

int main(void)
{
    // co_return tests
    {
        std::cout << "--- co_return [init_suspend:Y, final_suspend:Y]\n";
        auto r = co_return_1();
        std::cout << "  [init suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.resume();
        std::cout << "  [final suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.destroy();
    }

    {
        std::cout << "\n--- co_return [init_suspend:N, final_suspend:N]\n";
        auto r = co_return_2();
        std::cout << "  [coroutine destroyed] done:" <<
            r.done() << ", ret_code: " << r.ret_code() << "\n";
    }

    // co_await tests
    {
        std::cout << "\n--- co_await "
            "[init suspend:N, final_suspend:Y, await_ready:N, await_suspend:Y]\n";
        auto r = co_await_1();
        std::cout << "  [co_await suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.resume();
        std::cout << "  [final suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.destroy();
    }

    {
        std::cout << "\n--- co_await "
            "[init suspend:Y, final_suspend:N, await_ready:Y, await_suspend:Y]\n";
        auto r = co_await_2();
        std::cout << "  [init suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.resume();
        std::cout << "  [coroutine destroyed] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.destroy();
    }

    {
        std::cout << "\n--- co_await "
            "[init suspend:N, final_suspend:N, await_ready:N, await_suspend:N]\n";
        auto r = co_await_3();
        std::cout << "  [coroutine destroyed] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
    }

    // co_yield tests
    {
        std::cout << "\n--- co_yield [init suspend:N, final_suspend:Y]\n";
        auto r = co_yield_1();
        while (!r.done()) {
            std::cout << "  [co_yield suspended] done:" <<
                r.done() << ", yield_code:" << r.yield_code() << "\n";
            r.resume();
        }
        std::cout << "  [final suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.destroy();
    }

    return 0;
}
