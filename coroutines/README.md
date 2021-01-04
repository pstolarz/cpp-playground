## Helper objects

* `promise`, `handle`: promise, coroutine handle associated with a coroutine.

* `awaitable`: result of applying `promise.await_transofrm()` on `co_await`
   expression.

* `awaiter`: result of applying `operator co_await()` on `awaitable`;
   `awaiter == awaitable` if the operator is not provided.

* `SETJMP`, `LONGJMP`: saves/restores coroutine execution context (processor
   registers, exception handler etc.) in/from its `handle`. Since C++ coroutines
   are stackless, they share common execution stack. For this reason
   stack-pointer register (e.g. RSP/ESP) is not preserved/restored by these
   routines (contrary to `longjmp(3)`, `setjmp(3)` which are able to switch
   the stack). Stack frame register  (e.g. RBP/EBP) is preserved/restored as
   other general purpose registers. It's worth to note the stack frame is not
   used to store (persistent) local variables of coroutine since they are
   located on a separate space on the heap (in so called coroutine state).

## Coroutine call pseudo code

Function type: regular (return address on the stack)

```c++
result_t coroutine_entry(args...)
{
    // Allocate coroutine state on the heap (most cases) or stack (in case of
    // some optimization). coroutine handle is usually represented 1:1 by this
    // state (assumed here). The state is responsible to store coroutine
    // promise object, call parameters, local variables, excecution state.
    //
    // Depending on context it translates to one of the following calls:
    // 1. promise_t::operator new(size);
    // 2. promise_t::operator new(size, args...);
    // 3. ::operator new(size);
    auto handle = alloc_coroutine_state(size);

    // Copy function parameters in the handle.
    copy_args(handle, args...);

    // Construct local objects in the handle.
    init_locals(handle);

    // Construct promise object in the handle.
    promise_t& promise = init_promise(handle);

    // Get return object representing the coroutine interface for the caller
    auto result = promise.get_return_object();

    // Run coroutine body provided by the user (see below).
    // The routine returns at coroutine's suspenstion point (co_await returns)
    // or coroutine completion (coroutine_body() returns).
    coroutine_body(promise);

    return result;
}

void coroutine_body(promise_t& promise)
{
    auto handle = coroutine_handle<promise_t>::from_promise(promise);

    // initial suspend (coroutine opening bracket)
    co_await promise.inital_suspend();

    try {
        // coroutine body
    }
    catch(...) {
        promise.unhandled_exception();
    }

final_suspend:
    // final suspend (coroutine closing bracket)
    co_await promise.final_suspend();

    // In case final_suspend() doesn't suspend, the corutinse is destroyed here.
    // Otherwise no std::coroutine_handle::resume() is possible - coroutine is
    // freed by calling std::coroutine_handle::destroy() by a coroutine's caller.
    //
    // std::coroutine_handle::destroy() frees the coroutine by:
    // 1. Calls the destructor of the promise object.
    // 2. Calls the destructors of the coroutine locals (still in scope).
    // 3. Calls the destructors of the function parameter copies.
    // 4. Calls operator delete (global or promise_t overloaded) to free
    //    the memory used by the coroutine state.
    handle.destroy();
}
```

## `co_await` pseudo code

Function type: inline (`std::coroutine_handle::resume()` return address on the stack)

```c++
if (!awaiter.is_ready())
{
    if (!SETJMP(handle))
    {
        // co_await's suspension point created; the coroutine is possible to
        // be resumed via std::coroutine_handle::resume()

        auto result = awaiter.await_suspend(handle);
        using result_type == decltype(result);

        if constexpr (
            (std::is_same_v<result_type, void>) ||
            (std::is_same_v<result_type, bool> && result == true) ||
            (std::is_same_v<result_type, std::noop_coroutine_handle>))
        {
            // return from std::coroutine_handle::resume() call;
            // Note, co_await is not yet finished therefore its value is not
            // yet ready.
            return;
        } else
        if constexpr ((std::is_same_v<result_type, std::coroutine_handle>))
        {
            // In case resuming is done on the current coroutine there is no
            // sense to resume via LONGJMP(), since this has the same effect
            // as leaving current if-statement. This is equivalent to return
            // false by await_suspend() for non-symmetric transfer.
            if (result != handle)
            {
                // Leave current exception handling block and link to the previous
                // one. This way symmetrically resumed coroutine will link with
                // its own exception handler to the handler previous to the
                // current one.
                leave_exception_block();

                // Symmetric transfer: continue by resuming other coroutine.
                // Note a difference between LONGJMP() and std::coroutine_handle::resume()
                // call. In the latter case returning from the other coroutine
                // would finish in the coroutine containing this co_await
                // statement.
                LONGJMP(result);
            }
        }
    }

    // Resume point.
    //
    // Note, the point may be reached not only via call to std::coroutine_handle::resume()
    // but also if await_suspend() returns false or (for symmetric transfer)
    // a handle of current coroutine.
}

// co_await processing finished;
// co_await expression value is ready
auto co_await_result = awaiter.await_resume();
```

## `co_yield`

`co_yield expr` is equivalent to:

```c++
co_await promise.yield_value(expr);
```

## `co_return` pseudo code

Function type: inline (as for `co_await`)

```c++
// co_return expr
using expr_type = decltype(expr);

if constexpr (std::is_same_v<expr_type, void>) {
    promise.return_void();
} else {
    promise.return_value(expr);
}

// see above
goto final_suspend;
```

## `std::coroutine_handle::resume()` pseudo code

Function type: regular

```c++
// Note, return from resume() is possible only via co_await's return.
LONGJMP(handle);
```
