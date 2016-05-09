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
        std::mutex& getLock() { return m_mtx; }

        void setCounter(int c) { counter = c; }
        int getCounter() const { return counter; }
        void setHV(int hv_value) { m_hv = hv_value; }
        int getHV() const { return m_hv; }

    private:

        std::atomic<int> counter;
        std::atomic<int> m_hv;

        std::mutex m_mtx;
};
