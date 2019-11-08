#include <iostream>
#include <vector>
#include <chrono>
#include <atomic>
#include <assert.h>
#include "DynamicQueue.h"
#include "StaticMutexQueue.h"
#include "StaticAtomicQueue.h"

const int TASK_NUM = 4 * 1024 * 1024;
const int PRODUCER_NUM[3]{1, 2, 4};
const int CONSUMER_NUM[3]{1, 2, 4};

template <typename T>
void producer(Queue<T> &q)
{
    for (int i = 0; i < TASK_NUM; i++)
        q.push(1);
}

template <typename T>
void consumer(Queue<T> &q, std::atomic<T> &sum, int producer_num, int consumer_num)
{
    T val;
    for (int i = 0; i < TASK_NUM * producer_num / consumer_num; i++)
    {
        while (!q.pop(val))
            ;
        sum += val;
    }
}

template <typename T>
void test_queue(Queue<T> &q, int producer_num, int consumer_num)
{
    std::vector<std::thread> prod_thr;
    std::vector<std::thread> cons_thr;
    std::atomic<T> sum(0);

    for (int i = 0; i < producer_num; i++)
        prod_thr.push_back(std::thread(producer<T>, std::ref(q)));
    for (int i = 0; i < consumer_num; i++)
        cons_thr.push_back(std::thread(consumer<T>, std::ref(q), std::ref(sum), producer_num, consumer_num));
    for (int i = 0; i < producer_num; i++)
        if (prod_thr[i].joinable())
            prod_thr[i].join();
    for (int i = 0; i < consumer_num; i++)
        if (cons_thr[i].joinable())
            cons_thr[i].join();
    //check sum
    if (sum != producer_num * TASK_NUM)
        throw;
}

template <typename T>
void time_queue(Queue<T> &q)
{
    for (int pr : PRODUCER_NUM)
    {
        std::cout << pr << " producers\n";
        for (int cons : CONSUMER_NUM)
        {
            auto begin = std::chrono::high_resolution_clock::now();
            test_queue<T>(q, pr, cons);
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed_s = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1e-6;
            std::cout << cons << " consumers: " << elapsed_s << '\n';
        }
    }
}

int main()
{
    int task = 2;

    const int queue_size[3]{1, 4, 16};
    switch (task)
    {
    case 1:
    {
        DynamicQueue<int> q;
        time_queue<int>(q);
    }
    break;
    case 2:
    {
        for (int qz : queue_size)
        {
            std::cout << "queue size: " << qz << '\n'
                      << '\n';
            StaticMutexQueue<int> q(qz);
            time_queue<int>(q);
        }
    }
    break;
    case 3:
    {
        for (int qz : queue_size) //qz = 1 takes a vey long time
        {
            std::cout << "queue size: " << qz << '\n'
                      << '\n';
            StaticAtomicQueue<int> q(qz);
            time_queue<int>(q);
        }
    }
    }

    return 0;
}