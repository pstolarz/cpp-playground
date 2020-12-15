#include <coroutine>
#include <exception>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>

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

enum class RaiseExcept: int
{
    EXCEPT_NO = 0,
    EXCEPT_INIT,        // exception raised in initial_suspend()
    EXCEPT_FINAL        // exception raised in final_suspend()
};

template<typename InitialSuspend, typename FinalSuspend>
struct Result
{
    struct Promise
    {
        // supported since gcc-11
        // using enum RaiseExcept;

        Promise() noexcept:
            _ready(false), _suspend(true),
            _rexcept(RaiseExcept::EXCEPT_NO), _results()
        {
            std::cout << "Promise::Promise(); " << (void*)this << "\n";
        }

        Promise(bool ready, bool suspend,
            RaiseExcept rexcept = RaiseExcept::EXCEPT_NO) noexcept:
            _ready(ready), _suspend(suspend), _rexcept(rexcept), _results()
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

        InitialSuspend initial_suspend() const
        {
            std::cout << "Promise::initial_suspend()\n";

            if (_rexcept == RaiseExcept::EXCEPT_INIT)
                throw std::runtime_error("init_suspend() exception");

            return {};
        }

        FinalSuspend final_suspend() const
        {
            std::cout << "Promise::final_suspend()\n";

            if (_rexcept == RaiseExcept::EXCEPT_FINAL)
                throw std::runtime_error("init_final() exception");

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

        void unhandled_exception() {
            std::cout << "Promise::unhandled_exception()\n";

            // re-throw the exception
            std::rethrow_exception(std::current_exception());
        }

    private:
        bool _ready;
        bool _suspend;
        RaiseExcept _rexcept;
        std::set<Result*> _results;

        friend struct Result;
    };

    using promise_type = Promise;
    using Handle = std::coroutine_handle<Promise>;

    Result(Promise *promise) noexcept:
        _promise(promise), _ret_code(), _yield_code(0)
    {
        std::cout << "Result::Result(); " << (void*)this << "\n";

         // bind the object with its promise
        _promise->_results.insert(this);
    }

    ~Result() {
        std::cout << "Result::~Result(); " << (void*)this << "\n";

        if (_promise) {
            // unbind the object from its promise
            _promise->_results.erase(this);

            // destroy promise if no more results are associated with it
            if (_promise->_results.size() <= 0)
                Handle::from_promise(*_promise).destroy();
        }
    }

    auto ret_code() {
        return _ret_code;
    }

    auto yield_code() {
        return _yield_code;
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

ResultAA co_return_1(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND)
{
    co_return 1;
}

ResultNN co_return_2(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND)
{
    co_return 2;
}

ResultNA co_await_1(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND)
{
    int ares = co_await 1;
    std::cout << "co_await -> " << ares << "\n";
    // co_return;
}

ResultAN co_await_2(bool = AWAIT_READY, bool = AWAIT_SUSPEND)
{
    int ares = co_await 2;
    std::cout << "co_await -> " << ares << "\n";
    co_return ares;
}

ResultNN co_await_3(bool = AWAIT_NOT_READY, bool = AWAIT_NOT_SUSPEND)
{
    int ares = co_await 3;
    std::cout << "co_await -> " << ares << "\n";
    co_return ares;
}

ResultNA co_yield_1()
{
    int i=0, yres;

    do {
        yres = co_yield ++i;
        std::cout << "co_yield -> " << yres << "\n";
    } while (yres < 5);

    co_return 1;
}

ResultNN co_except_1(bool = AWAIT_READY, bool = AWAIT_NOT_SUSPEND,
    RaiseExcept = RaiseExcept::EXCEPT_NO)
{
    throw std::runtime_error("coroutine exception");
    co_return 1;
}

ResultNN co_except_2(bool = AWAIT_READY, bool = AWAIT_NOT_SUSPEND,
    RaiseExcept = RaiseExcept::EXCEPT_INIT)
{
    co_return 2;
}

ResultNN co_except_3(bool = AWAIT_READY, bool = AWAIT_NOT_SUSPEND,
    RaiseExcept = RaiseExcept::EXCEPT_FINAL)
{
    co_return 3;
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
        std::cout << "  [coroutine destroyed via RAII]\n";
    }

    {
        std::cout << "\n--- co_return [init_suspend:N, final_suspend:N]\n";
        auto r = co_return_2();
        std::cout << "  [coroutine destroyed automatically] done:" <<
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
        std::cout << "  [coroutine destroyed via RAII]\n";
    }

    {
        std::cout << "\n--- co_await "
            "[init suspend:Y, final_suspend:N, await_ready:Y, await_suspend:Y]\n";
        auto r = co_await_2();
        std::cout << "  [init suspended] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
        r.resume();
        std::cout << "  [coroutine destroyed automatically] done:" <<
            r.done() << ", ret_code:" << r.ret_code() << "\n";
    }

    {
        std::cout << "\n--- co_await "
            "[init suspend:N, final_suspend:N, await_ready:N, await_suspend:N]\n";
        auto r = co_await_3();
        std::cout << "  [coroutine destroyed automatically] done:" <<
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
    }

    // exception tests
    //
    // BUG in gcc-10: while raising exception on any stage of coroutine life
    // (e.g. initial/final suspend, coroutine body) the coroutine frame is not
    // freed and the stack with coroutine result object is not unwinded:
    //
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95615
    {
        std::cout << "\n--- exception in coroutine body\n";
        try {
            auto r = co_except_1();
        } catch(const std::exception& e) {
            std::cout << "Exception: " << e.what() << "\n";
        }
    }

    {
        std::cout << "\n--- exception in initial_suspend()\n";
        try {
            auto r = co_except_2();
        } catch(const std::exception& e) {
            std::cout << "Exception: " << e.what() << "\n";
        }
    }

    {
        std::cout << "\n--- exception in final_suspend()\n";
        try {
            auto r = co_except_3();
        } catch(const std::exception& e) {
            std::cout << "Exception: " << e.what() << "\n";
        }
    }

    return 0;
}
