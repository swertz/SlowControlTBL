#pragma once

#include <mutex>
#include <atomic>
#include <thread>
#include <iostream>
#include <utility>
#include <vector>
#include <string>

class Setup {
    
    public:

        Setup(): 
            counter(0), 
            m_state(State::idle)
        {
            std::cout << "Creating Setup" << std::endl;

            // for testing purposes
            setState(State::configured);

            startDaemons();
        }

        ~Setup() { stopDaemons(); }

        /*
         * Define states of the state machine.
         * Possible transitions (defined in constructor):
         *  idle -> configured
         *  configured -> running
         *  running -> idle
         *  configured -> idle
         */
        enum class State {
            idle,
            configured,
            running
        };

        /*
         * Get current Setup state
         */
        Setup::State getState() const { return m_state; }
        
        /*
         * Change Setup state to `state`.
         * Throws exception if transition from current state is not allowed.
         * Returns:
         *  - `true` if state was changed successfully
         *  - `false` if Setup was already in the requested state
         */
        bool setState(Setup::State state);
        
        /*
         * Convert the state to a string
         */
        static std::string stateToString(Setup::State state);

        /*
         * Check if transition from `state_from` to `state_to` is allowed
         */
        static bool checkTransition(Setup::State state_from, Setup::State state_to);
        
        void lock() { m_mtx.lock(); }
        void unlock() { m_mtx.unlock(); }
        std::mutex& getLock() { return m_mtx; }

        void setCounter(int c) { counter = c; }
        int getCounter() const { return counter; }
        void setHV(int hv_value) { m_hv = hv_value; }
        int getHV() const { return m_hv; }

        /*
         * Daemons: will run as threads in the background,
         * handle the HV & TDC cards
         */
        void daemonHV();
        void daemonTDC();

        void startDaemons();
        void stopDaemons();
       
        // Transitions are defined in .cc file
        static const std::vector< std::pair<State, State> > m_transitions;

    private:

        std::atomic<int> counter;
        std::atomic<int> m_hv;

        std::mutex m_mtx;

        std::atomic<State> m_state;

        std::thread thread_handle_HV;
        std::thread thread_handle_TDC;
};
