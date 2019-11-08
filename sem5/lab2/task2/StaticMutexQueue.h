#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include "Queue.h"

template <typename T>
class StaticMutexQueue : public Queue<T>
{
    T *q;
    const size_t size;
    size_t current_size, head_ind, tail_ind;
    std::mutex mtx;
    std::condition_variable cv;

public:
    StaticMutexQueue(size_t size) : size(size), current_size(0), head_ind(0), tail_ind(0)
    {
        q = new T[size]{T()};
    }
    ~StaticMutexQueue()
    {
        delete[] q;
    }
    void push(T val) override
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return current_size < size; });
        q[tail_ind] = val;
        tail_ind = (tail_ind + 1) % size;
        current_size++;
        // Manual unlocking is done before notifying, to avoid waking up
        // the waiting thread only to block again (see notify_one for details)
        //lock.unlock();
        cv.notify_one();
    }
    bool pop(T &val) override
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.notify_all();
        if (cv.wait_for(lock, std::chrono::milliseconds(1), [&] { return current_size > 0; }))
        {
            val = q[head_ind];
            q[head_ind] = 0;
            head_ind = (head_ind + 1) % size;
            current_size--;
            //lock.unlock();
            cv.notify_one();
            return true;
        }
        else
            return false;
    }
};