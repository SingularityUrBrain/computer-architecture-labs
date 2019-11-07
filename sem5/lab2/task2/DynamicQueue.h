#include <thread>
#include <mutex>
#include <queue>
#include "Queue.h"


template <typename T>
class DynamicQueue : public Queue<T>
{
    std::mutex mtx;
    std::queue<T> q;

public:
    void push(T val) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(val);
    }
    bool pop(T& val) override
    {
        mtx.lock();
        if (q.empty())
        {
            mtx.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            mtx.lock();
            if (q.empty())
            {
                mtx.unlock();
                return false;
            }
        }
        val = q.front();
        q.pop();
        mtx.unlock();
        return true;
    }
};