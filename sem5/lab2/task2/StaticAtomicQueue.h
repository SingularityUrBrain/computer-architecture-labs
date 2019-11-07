#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include "Queue.h"

template <typename T>
class StaticAtomicQueue : public Queue<T>
{
    const size_t size;
    std::atomic<size_t> head, tail;
    std::vector<T> q;

public:
    StaticAtomicQueue(size_t size) : size(size), head(0), tail(0)
    {
        q.resize(size);
    }

    void push(T val) override
    {
        while (1)
        {
            size_t curr_tail = tail.load();
            if (curr_tail == head + size) // overf;ow check
                continue;
            if (tail.compare_exchange_strong(curr_tail, curr_tail + 1))
            {
                q[curr_tail % size] = val;
                return;
            }
        }
    }

    bool pop(T &val) override
    {
        while (1)
        {
            size_t curr_head = head.load();
            if (curr_head == tail)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                curr_head = head.load();
                if (curr_head == tail)
                    return false;
            }
            if (head.compare_exchange_strong(curr_head, curr_head + 1))
            {
                val = q[curr_head % size];
                return true;
            }
        }
    }
};