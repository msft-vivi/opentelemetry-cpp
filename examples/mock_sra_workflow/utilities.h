#pragma once

#include <iostream>
#include <functional>
#include <utility>

// Function wrapper
template <typename Function>
class Functor
{
public:
    Functor(Function p_func) : m_func(p_func) {}
    Function& operator()()
    {
        return m_func;
    }

private:
    Function m_func;
};

struct RetryUtility
{
    template <typename Function, typename... Args>
    static bool RunOnFailureRetry(Function p_func, uint32_t p_retryTimes, uint32_t p_retryInterval, Args&&... args)
    {
        for (size_t attempt = 0; attempt< p_retryTimes + 1; ++attempt)
        {
            if (p_func(std::forward<Args>(args)...))
            {
                std::cout << "success" << std::endl;
                return true;
            }
            else
            {
                std::cout << "retry" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(p_retryInterval));
            }
        }

        return false;
    };
};