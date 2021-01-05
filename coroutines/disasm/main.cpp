#include <coroutine>
#include <cstdio>

template<typename Promise>
struct Awaitable
{
    Awaitable(bool ready, bool suspend, int resume) noexcept:
        _ready(ready), _suspend(suspend), _resume(resume)
    {
        printf("Awaitable::Awaitable(); %p\n", (void*)this);
    }

    ~Awaitable() noexcept {
        printf("Awaitable::~Awaitable(); %p\n", (void*)this);
    }

    bool await_ready() const noexcept {
        printf("Awaitable::await_ready() -> %d\n", _ready);
        return _ready;
    }

    bool await_suspend(std::coroutine_handle<Promise> h) const noexcept {
        printf("Awaitable::await_suspend() -> %d\n", _suspend);
        return _suspend;
    }

    int await_resume() const noexcept {
        printf("Awaitable::await_resume() -> %d\n", _resume);
        return _resume;
    }

private:
    bool _ready;
    bool _suspend;
    int _resume;
};

template<typename InitialSuspend, typename FinalSuspend>
struct Result
{
    struct Promise
    {
        Promise() noexcept: _ready(false), _suspend(true) {
            printf("Promise::Promise(); %p\n", (void*)this);
        }

        Promise(bool ready, bool suspend) noexcept:
            _ready(ready), _suspend(suspend)
        {
            printf("Promise::Promise(); %p\n", (void*)this);
        }

        ~Promise() noexcept {
            printf("Promise::~Promise(); %p\n", (void*)this);
        }

        Awaitable<Promise> await_transform(int resume) const noexcept {
            printf("Promise::await_transform()\n");
            return {_ready, _suspend, resume};
        }

        Result get_return_object() noexcept {
            printf("Promise::get_return_object()\n");
            return *this;
        }

        InitialSuspend initial_suspend() const {
            printf("Promise::initial_suspend()\n");
            return {};
        }

        FinalSuspend final_suspend() const {
            printf("Promise::final_suspend()\n");
            return {};
        }

        void return_void() const noexcept {
            printf("Promise::return_void()\n");
        }

        void return_value(int ret_code) noexcept {
            printf("Promise::return_value(); value: %d\n", ret_code);
        }

        void unhandled_exception() {
            printf("Promise::unhandled_exception()\n");
            throw;
        }

    private:
        bool _ready;
        bool _suspend;

        friend struct Result;
    };

    using promise_type = Promise;
    using Handle = std::coroutine_handle<Promise>;

    Result() = delete;

    Result(Promise& promise) noexcept: _promise(promise) {
        printf("Result::Result(); %p\n", (void*)this);
    }

    ~Result() {
        printf("Result::~Result(); %p\n", (void*)this);
        Handle::from_promise(_promise).destroy();
    }

    Result(const Result& r) noexcept: Result(r._promise) {}
    Result(Result&& r) noexcept: Result(r) {}

    Result& operator=(const Result&) = delete;
    Result& operator=(const Result&&) = delete;

    bool done() {
        return Handle::from_promise(_promise).done();
    }

    void resume() {
        Handle::from_promise(_promise).resume();
    }

private:
    Promise& _promise;
};


constexpr const bool AWAIT_READY       = true;
constexpr const bool AWAIT_NOT_READY   = false;
constexpr const bool AWAIT_SUSPEND     = true;
constexpr const bool AWAIT_NOT_SUSPEND = false;

using ResultAA = Result<std::suspend_always, std::suspend_always>;
using ResultAN = Result<std::suspend_always, std::suspend_never>;
using ResultNN = Result<std::suspend_never, std::suspend_never>;
using ResultNA = Result<std::suspend_never, std::suspend_always>;

ResultAA co_await_test(bool = AWAIT_NOT_READY, bool = AWAIT_SUSPEND)
{
    int ares = co_await 1;
    printf("co_await -> %d\n", ares);
    co_return 1;
}

int main(void)
{
    auto r = co_await_test();

    printf("  [initially suspended] done:%d\n", r.done());
    r.resume();

    printf("  [co_await suspended] done:%d\n", r.done());
    r.resume();

    printf("  [finally suspended] done:%d\n", r.done());
    return 0;
}
