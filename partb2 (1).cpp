#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <ctime>
#include <atomic>

const int kNumThreads = 4;
const int kNumIterations = 100000;

std::vector<int> shared_vector;
std::mutex mtx;
std::atomic_flag m2;
std::atomic_flag timey;

int count = 0;

void worker_thread() {
    while(timey.test_and_set());
    auto start4 = std::chrono::high_resolution_clock::now();
    timey.clear();

    while(m2.test_and_set());
    for (int i = 0; i < kNumIterations; ++i) {  
        // std::lock_guard<std::mutex> lock(mtx);
        shared_vector.push_back(i);
        // count+=std::chrono::duration_cast<std::chrono::microseconds>(end4 - start4).count();
    }
    m2.clear();

    while(timey.test_and_set());
    auto end4 = std::chrono::high_resolution_clock::now();
    timey.clear();
    
    std::cout<<"sync overhead w primitive: "<< std::chrono::duration_cast<std::chrono::microseconds>(end4 - start4).count()<<std::endl;
}

int main() {
    std::vector<std::thread> threads;

    auto start1 = std::chrono::high_resolution_clock::now();
    auto clock1 = std::clock();
    // TODO: Measure time for creating threads
    for (int i = 0; i < kNumThreads; ++i) {
        auto start3 = std::chrono::high_resolution_clock::now();
        auto clock3 = std::clock();
        threads.emplace_back(worker_thread);
        auto end3 = std::chrono::high_resolution_clock::now();
        auto endclock3 = std::clock();

        std::cout<< "time to create thread: " << std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3).count() << std::endl;
        std::cout<< "time to create thread(clock): " << endclock3 - clock3 << std::endl;
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto endclock1 = std::clock();

    std::cout<< "loop w threads: " << std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() << std::endl;

    std::cout<< "loop w threads(clock): " << endclock1-clock1<<std::endl;

    // Clear shared_vector for the next measurement
    shared_vector.clear();

    auto start2 = std::chrono::high_resolution_clock::now();
    auto clock2 = std::clock();
    for (int i = 0; i < kNumThreads * kNumIterations; ++i) {
        shared_vector.push_back(i);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto endclock2 = std::clock();

    std::cout<< "loop w/o threads: " << std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count() << std::endl;
    std::cout<< "loop w/o threads(clock): " << endclock2-clock2 << std::endl;

    std::cout<<"mutex overhead: "<<count/kNumThreads<<std::endl;

    return 0;
}
