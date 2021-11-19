#include <coroutine>
#include <iostream>
#include <string>
#include <vector>

template<typename Promise>
struct Awaitable
{
    using Handle = std::coroutine_handle<Promise>;

    Awaitable(Promise *current, Promise *next)
        noexcept: _current(current), _next(next)
    {
        std::cout << "[" << (void*)this << "] Awaitable::Awaitable() [" <<
            _current->_fname << "]\n";
    }

    Awaitable(Promise&& next) noexcept: Awaitable(next) {}

    ~Awaitable() noexcept {
        std::cout << "[" << (void*)this << "] Awaitable::~Awaitable()\n";
    }

    bool await_ready() const noexcept {
        std::cout << "[" << (void*)this << "] Awaitable::await_ready() -> 0\n";
        // always suspend
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const noexcept
    {
        if (!_next) {
            std::cout << "[" << (void*)this <<
                "] Awaitable::await_suspend(); AWAIT_NOOP\n";

            return std::noop_coroutine();
        } else
        if (_next == _current) {
            std::cout << "[" << (void*)this <<
                "] Awaitable::await_suspend(); AWAIT_RESUME\n";

            return Handle::from_promise(*_current);
        } else {
            std::cout << "[" << (void*)this << "] Awaitable::await_suspend() " <<
                "switched to " << _next->_fname << "\n";

            // switch to the next co-routine
            return Handle::from_promise(*_next);
        }
    }

    int await_resume() const noexcept
    {
        if (!_next) {
            std::cout << "[" << (void*)this <<
                "] Awaitable::await_resume() -> 0; AWAIT_NOOP\n";
        } else
        if (_next == _current) {
            std::cout << "[" << (void*)this <<
                "] Awaitable::await_resume() -> 0; AWAIT_RESUME\n";
        } else
        {
            bool next_done = Handle::from_promise(*_next).done();

            std::cout << "[" << (void*)this << "] Awaitable::await_resume() -> " <<
                _next->_ret_code << "; " << _next->_fname << ", done:" <<
                next_done << "\n";

            // in case next coroutine is done no more switch to it is possible
            if (next_done) _current->_next = nullptr;

            return _next->_ret_code;
        }

        return 0;
    }

private:
    Promise *_current;
    Promise *_next;
};

struct Result
{
    struct Promise
    {
        Promise(std::string fname,
            [[maybe_unused]] int = 0 /* reserved for coroutine extra param. */)
            noexcept: _fname(fname)
        {
            std::cout << "[" << (void*)this << "] Promise::Promise() [" <<
                _fname << "]\n";
        }

        ~Promise() noexcept {
            std::cout << "[" << (void*)this << "] Promise::~Promise()\n";
            _destr_cnt++;
        }

        Awaitable<Promise> await_transform(Result& result) noexcept {
            _next = result._promise;
            return {this,
                (!result._promise && !result._noop ? this : result._promise)};
        }

        Awaitable<Promise> await_transform(Result&& result) noexcept {
            return await_transform(result);
        }

        // for in-place suspending tests
        std::suspend_always await_transform(
            [[maybe_unused]] int /* unused, co_await requires expr. */) noexcept
        {
            return {};
        }

        Result get_return_object() noexcept {
            std::cout << "[" << (void*)this << "] Promise::get_return_object()\n";
            return {this};
        }

        /*
         * Need to be initially resumed since newly created coroutine is
         * resumed by symmetric co_await.
         */
        std::suspend_always initial_suspend() const {
            std::cout << "[" << (void*)this <<
                "] Promise::initial_suspend() [always]\n";
            return {};
        }

        /*
         * Final suspending guarantees the result object is always associated
         * with a valid promise, that is the promise is never destroyed before
         * its result object. Promise will be freed only in case the last result
         * object referenced to it is destroyed.
         */
        std::suspend_always final_suspend() const noexcept {
            std::cout << "[" << (void*)this <<
                "] Promise::final_suspend() [always]\n";
            return {};
        }

        void return_value(int ret_code) noexcept {
            std::cout << "[" << (void*)this << "] Promise::return_value()\n";
            _ret_code = ret_code;
        }

        void unhandled_exception() {
            std::cout << "[" << (void*)this << "] Promise::unhandled_exception()\n";
            // re-throw the exception
            throw;
        }

        // coroutine handle object allocation
        void *operator new(size_t sz) {
            void *ptr = malloc(sz);
            std::cout << "Promise::operator new(); " << ptr << "\n";

            return ptr;
        }

        // coroutine handle object destruction
        void operator delete(void *ptr) {
            std::cout << "Promise::operator delete() " << ptr << "\n";

            Promise& promise = Handle::from_address(ptr).promise();
            if (promise._destr_cnt <= 1) {
                // double free protection
                free(ptr);
            }
        }
    private:
        std::string _fname;

        // coroutine next to this one in the switching chain
        Promise *_next = nullptr;

        unsigned _ref_cnt = 0;
        int _ret_code = -1;
        int _destr_cnt = 0;

    friend struct Result;
    friend struct Awaitable<Promise>;
    };

    using promise_type = Promise;
    using Handle = std::coroutine_handle<Promise>;

    Result(Promise *promise) noexcept: _promise(promise) {
        std::cout << "[" << (void*)this << "] Result::Result() [" <<
            _promise->_fname << "]\n";

        // mark reference with promise
        _promise->_ref_cnt++;
    }

    ~Result() noexcept {
        std::cout << "[" << (void*)this << "] Result::~Result()\n";

        if (_promise) {
            // unreference promise
            _promise->_ref_cnt--;

            // destroy promise if no more results reference it
            if (_promise->_ref_cnt <= 0)
                Handle::from_promise(*_promise).destroy();
        }
    }

    Result() = delete;

    Result(const Result& r) noexcept: Result(r._promise) {}
    Result(Result&& r) noexcept: Result(r) {}

    Result& operator=(const Result&) = delete;
    Result& operator=(const Result&&) = delete;

    int ret_code() {
        return _promise->_ret_code;
    }

    /*
     * Resume most nested coroutine in a chain of calls.
     *
     * In case the coroutine to resume is finally suspended its resumption is
     * not possible. In this case its caller is resumed and the process of
     * resumption is repeated on the resulting chain. This allows to finish
     * all finally suspended coroutines in the chain via RAII (calling suspended
     * coroutines result objects destructors).
     */
    void resume()
    {
        bool fsusp;
        Promise *to_resume, *to_resume_prev = nullptr;

        for (;;) {
            fsusp = false;
            to_resume = nullptr;

            for (Promise *p = _promise; p; p = p->_next) {
                if (!Handle::from_promise(*p).done())
                    to_resume = p;
                else
                    fsusp = true;
            }

            if (to_resume) {
                std::cout << "[" << (void*)this << "] Result::resume() resumes "
                    << to_resume->_fname << "\n";

                Handle::from_promise(*to_resume).resume();
            }

            if (!to_resume || !fsusp || to_resume == to_resume_prev)
                break;

            to_resume_prev = to_resume;
        }
    }

    bool done() {
        return Handle::from_promise(*_promise).done();
    }

    // special co_await arg.: NOOP resume
    static Result AWAIT_NOOP() {
        return {true};
    }

    // special co_await arg.: resume on the same coroutine
    static Result AWAIT_RESUME() {
        return {false};
    }

private:
    Result(bool noop): _noop(noop) {}

    Promise *_promise = nullptr;
    bool _noop = false;
};


Result T1_f(std::string fname = "T1_f") {
    std::cout << "  " << fname << " co_await [suspending]\n";
    co_await Result::AWAIT_NOOP();

    std::cout << "  " << fname << " co_return -> 1\n";
    co_return 1;
}


Result T2_f(std::string fname = "T2_f") {
    std::cout << "  " << fname << " co_await [suspending]\n";
    co_await Result::AWAIT_RESUME();

    std::cout << "  " << fname << " co_return -> 2\n";
    co_return 2;
}


template<int I>
Result T3_f(std::string fname) {
    auto rc = co_await T3_f<I-1>("T3_f<" + std::to_string(I-1) + ">");
    std::cout << "  " << fname << " co_await -> " << rc << "\n";
    co_return rc;
}

template<>
Result T3_f<1>(std::string fname) {
    std::cout << "  " << fname << " co_return -> 3\n";
    co_return 3;
}


Result T4_f(std::string fname, int ret)
{
    std::cout << "  " << fname << " co_await [suspending]\n";
    co_await 0; // co_await expr. (0) unused

    std::cout << "  " << fname << " co_return -> " << ret << "\n";
    co_return ret;
}

Result T4_f1(std::string fname = "T4_f1")
{
    std::vector<Result> v = {
        T4_f("T4_f [1]", 1),
        T4_f("T4_f [2]", 2),
        T4_f("T4_f [3]", 3)
    };

    for (auto& r: v)
        co_await r;

    co_return 4;
}

int main(void)
{
    /*
     * Note on symmetric transfer and co_await context switch to a caller.
     * Assume the following chain of calls:
     *
     * caller (creates coroutine_1 and initially resumes it)
     * |
     * +- coroutine_1 (symmetric trans. to coroutine_2)
     *    |
     *    +- coroutine_2 (suspends on co_await or final stage)
     *
     * Then coroutine_2 suspension switches the context directly to the top-most
     * caller not to coroutine_1 which created coroutine_1 and resumed it
     * on its initial stage.
     *
     * This differs of the case where coroutine_2 was not to be symmetrically
     * transferred (e.g. created it and directly resumed), which would cause
     * switching the running context directly to coroutine_2's resumer (that
     * is coroutine_1).
     *
     * The reason of this differ is symmetric transfer resumes next coroutine
     * by JMP to its suspension point, while asymmetric transfer uses CALL for
     * this purpose. Therefore if a transfer is returned back to a resumer/caller
     * in co_await - this is done via RET command, therefore to the last
     * call of coroutine_handle<>.resume().
     */

    {
        std::cout << "\n--- noop_coroutine test\n";
        auto r = T1_f();

        std::cout << "  [initially suspended] done:" << r.done() <<"\n";
        r.resume();

        std::cout << "  [noop suspended] done:" << r.done() <<"\n";
        r.resume();
    }

    {
        std::cout << "\n--- resume current coroutine test\n";
        auto r = T2_f();

        std::cout << "  [initially suspended] done:" << r.done() <<"\n";
        r.resume();
    }

    {
        std::cout << "\n--- Chain of nested calls\n";
        auto r = T3_f<3>("T3_f<3>");

        std::cout << "  [initially suspended] done:" << r.done() <<"\n";
        r.resume();
        // see note above
        std::cout << "  [nested-call suspended] done:" << r.done() <<"\n";
        r.resume();
        std::cout << "  [finally suspended] done:" << r.done() <<
            ", ret_code:" << r.ret_code()  << "\n";
    }

    {
        std::cout << "\n--- Iterate over vector of coroutines calls\n";
        auto r = T4_f1();

        std::cout << "  [initially suspended] done:" << r.done() <<"\n";
        r.resume();

        // process resuming until coroutine is done
        int i = 0;
        do {
            std::cout << "  [resume no. " << ++i << "]\n";
            r.resume();
        } while (!r.done());

        std::cout << "  [finally suspended] done:" << r.done() <<
            ", ret_code:" << r.ret_code()  << "\n";
    }

    return 0;
}
