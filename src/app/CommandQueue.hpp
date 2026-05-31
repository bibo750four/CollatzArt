#pragma once
#include <queue>
#include <mutex>
#include <optional>

template<typename T>
class CommandQueue
{
public:
    // chiamato dal thread CLI
    void push(T cmd)
    {
        std::lock_guard lock(mutex_);
        queue_.push(std::move(cmd));
    }

    // chiamato dal thread principale nel render loop — non bloccante
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