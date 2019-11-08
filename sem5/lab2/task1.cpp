#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <atomic>
#include <functional>

const int NUM_TASKS = 1024 * 1024;
const int nums_thrs[4]{4, 8, 16, 32};
const int sleep[2]{0, 10};

std::mutex mtx;

void mutex_fill(int *arr, int &ind, size_t sleep_time)
{
    mtx.lock();
    int pind = ind++;
    mtx.unlock();
    while (pind < NUM_TASKS)
    {
        arr[pind]++;
        mtx.lock();
        pind = ind++;
        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
    }
}

void atomic_fill(int *arr, std::atomic<int> &ind, size_t sleep_time)
{
    int pind = ind++;
    while (pind < NUM_TASKS)
    {
        arr[pind]++;
        pind = ind++;
        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));
    }
}

template <typename T>
void run_filling(int num_threads, size_t sleep_time, T &ind, std::function<void(int *, T &, size_t)> fill)
{
    int *arr = new int[NUM_TASKS]{0};
    std::vector<std::thread> thread_v;

    for (int i = 0; i < num_threads; i++)
        thread_v.push_back(std::thread(fill, std::ref(arr), std::ref(ind), sleep_time));
    for (int i = 0; i < num_threads; i++)
        if (thread_v[i].joinable())
            thread_v[i].join();
    // check
    for (int i = 0; i < NUM_TASKS; i++)
        if (arr[i] != 1)
            throw;

    delete[] arr;
}

int main(int argc, char *argv[])
{
    for (int sl : sleep)
    {
        std::cout << "sleep: " << sl << '\n';
        for (int thr : nums_thrs)
        {
            int ind = 0;
            auto begin = std::chrono::high_resolution_clock::now();
            run_filling<int>(thr, sl, ind, mutex_fill);
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed_s1 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1e-6;

            std::atomic<int> ind_atomic(0);
            begin = std::chrono::high_resolution_clock::now();
            run_filling<std::atomic<int>>(thr, sl, ind_atomic, atomic_fill);
            end = std::chrono::high_resolution_clock::now();
            auto elapsed_s2 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1e-6;

            std::cout << thr << " threads" << '\n'
                      << "mutex: " << elapsed_s1 << "\natomic: " << elapsed_s2 << '\n';
        }
        std::cout << '\n';
    }
    return 0;
}