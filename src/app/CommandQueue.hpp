#pragma once
#include <queue>
#include <mutex>
#include <optional>

template<typename T>
class CommandQueue
{
public:
    // Called from the CLI thread
    void push(T cmd)
    {
        std::lock_guard lock(mutex_);
        queue_.push(std::move(cmd));
    }

    // Called from the main render-loop thread — non-blocking
    std::optional<T> tryPop()
    {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) return std::nullopt;
        T cmd = std::move(queue_.front());
        queue_.pop();
        return cmd;
    }

private:
    std::queue<T>  queue_;
    std::mutex     mutex_;
};
