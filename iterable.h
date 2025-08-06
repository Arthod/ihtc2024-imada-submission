#ifndef ITERABLE_H
#define ITERABLE_H

#include <coroutine>
#include <optional>
#include <iostream>
#include <memory>
#include <vector>

#include "move.h"

template<typename T>
class Iterable {
public:
    struct promise_type {
        std::optional<T> current_value;

        Iterable get_return_object() { 
            return Iterable{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    Iterable(handle_type h) : handle(h) {}
    ~Iterable() { if (handle) handle.destroy(); }

    bool move_next() {
        if (!handle || handle.done()) return false;
        handle.resume();
        return !handle.done();
    }

    T current() {
        return std::move(handle.promise().current_value.value());
    }

private:
    handle_type handle;
};

#endif