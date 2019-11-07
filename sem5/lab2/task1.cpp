#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <atomic>
#include <assert.h>

const int NUM_TASKS = 1024 * 1024;
const int nums_thrs[4]{4, 8, 16, 32};
const int sleep[2]{0, 10};

std::mutex mtx;

void mutex_fill(int *arr, int &ind, size_t sleep_time)
{
    int pind;
    while (1)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            pind = ind++;
        }
        if (pind >= NUM_TASKS)
            return;
        arr[pind]++;
        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
    }
}

void mutex_task(int num_threads, size_t sleep_time)
{
    int *arr = new int[NUM_TASKS]{0};
    int ind = 0;
    std::vector<std::thread> thread_v;

    for (int i = 0; i < num_threads; i++)
        thread_v.push_back(std::thread(mutex_fill, std::ref(arr), std::ref(ind), sleep_time));
    for (int i = 0; i < num_threads; i++)
        if (thread_v[i].joinable())
            thread_v[i].join();
    // check
    for (int i = 0; i < NUM_TASKS; i++)
        assert(arr[i] == 1);

    delete[] arr;
}

void atomic_fill(int *arr, std::atomic<int> &ind, size_t sleep_time)
{
    int pind;
    while (1)
    {
        pind = ind++;
        if (pind >= NUM_TASKS)
            return;
        arr[pind]++;
        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
    }
}

void atomic_task(int num_threads, size_t sleep_time)
{
    int *arr = new int[NUM_TASKS]{0};
    std::atomic<int> ind(0);
    std::vector<std::thread> thread_v;

    for (int i = 0; i < num_threads; i++)
        thread_v.push_back(std::thread(atomic_fill, std::ref(arr), std::ref(ind), sleep_time));
    for (int i = 0; i < num_threads; i++)
        if (thread_v[i].joinable())
            thread_v[i].join();
    // check
    for (int i = 0; i < NUM_TASKS; i++)
        assert(arr[i] == 1);

    delete[] arr;
}

int main(int argc, char *argv[])
{
    for (int sl : sleep)
    {
        std::cout << "sleep: " << sl << '\n';
        for (int thr : nums_thrs)
        {
            auto begin = std::chrono::high_resolution_clock::now();
            mutex_task(thr, sl);
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed_s1 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1e-6;

            begin = std::chrono::high_resolution_clock::now();
            atomic_task(thr, sl);
            end = std::chrono::high_resolution_clock::now();
            auto elapsed_s2 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1e-6;
            std::cout << thr << " threads" << '\n'
                      << "mutex: " << elapsed_s1 << "\natomic: " << elapsed_s2 << '\n';
        }
        std::cout << '\n';
    }
    return 0;
}