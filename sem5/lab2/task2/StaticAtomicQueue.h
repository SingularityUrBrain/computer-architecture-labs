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
    std::atomic<bool> isbusy_push, isbusy_pop;
    std::vector<T> q;

public:
    StaticAtomicQueue(size_t size) : size(size), head(0), tail(0), isbusy_push(false), isbusy_pop(false)
    {
        q.resize(size);
    }

    void push(T val) override
    {
        while (1)
        {
            size_t curr_tail = tail.load();
            // check overflow
            if (curr_tail == head + size) continue;
            T x = q[curr_tail % size];
            if (curr_tail != tail || x!=0) continue;
            if(tail.compare_exchange_strong(curr_tail, curr_tail + 1))
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
            // empty
            if (curr_head == tail)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                curr_head = head.load();
                if (curr_head == tail)
                    return false;
            }
            T x = q[curr_head % size];
            if (curr_head != head || x == 0) continue;
            if (head.compare_exchange_strong(curr_head, curr_head+1))
            {
                val = x;
                q[curr_head % size] = 0;
                return true;
            }
        }
    }
};