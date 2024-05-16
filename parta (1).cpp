#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>




std::vector<int> vec;

std::mutex m1;

std::atomic_flag m2;
void increment() {
    for (int i = 0; i < 1000000; ++i) {
        // m1.lock();
        while(m2.test_and_set());
        vec.push_back(i);
        m2.clear();
        // m1.unlock();
    }
}



int main() {
    std::thread t1(increment);
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread t2(increment);
    t1.join();
    t2.join();
    std::cout << "Vector size: " << vec.size() << std::endl;
    return 0;
}


