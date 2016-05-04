#pragma once

#include <mutex>
#include <atomic>
#include <iostream>

class Setup {

    public:

        Setup(): counter(0) {
            std::cout << "Creating Setup." << std::endl;
        }

        void lock() { m_mtx.lock(); }
        void unlock() { m_mtx.unlock(); }

        void setCounter(int c) { counter = c; }
        int getCounter() { return counter; }

    private:

        std::atomic<int> counter;

        std::mutex m_mtx;
};
