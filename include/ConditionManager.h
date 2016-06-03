#pragma once

#include <mutex>
#include <atomic>
#include <thread>
#include <iostream>
#include <utility>
#include <vector>
#include <string>

class ConditionManager {
    
    public:

        ConditionManager(): 
            m_hvpmt_setStates({1, 1, 1, 0}),
            m_hvpmt_setValues({1025, 925, 1225, 0}),
            m_state(State::idle)
        {

            // for testing purposes
            setState(State::configured);

            startDaemons();
        }

        ~ConditionManager() { stopDaemons(); }

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
         * Get current ConditionManager state
         */
        ConditionManager::State getState() const { return m_state; }
        
        /*
         * Change ConditionManager state to `state`.
         * Throws exception if transition from current state is not allowed.
         * Returns:
         *  - `true` if state was changed successfully
         *  - `false` if ConditionManager was already in the requested state
         */
        bool setState(ConditionManager::State state);
        
        /*
         * Convert the state to a string
         */
        static std::string stateToString(ConditionManager::State state);

        /*
         * Check if transition from `state_from` to `state_to` is allowed
         */
        static bool checkTransition(ConditionManager::State state_from, ConditionManager::State state_to);
        
        void lock() { m_mtx.lock(); }
        void unlock() { m_mtx.unlock(); }
        std::mutex& getLock() { return m_mtx; }

        /*
         * Define/retrieve the PMT HV value (no action taken on the setup)
         * So far, this is a vector with entry==channel
         */
        void setHVPMTValue(int HVNumber, int HVValue);
        const std::vector<int>& getHVPMTSetValues() const { return m_hvpmt_setValues; }
        const std::vector<int>& getHVPMTSetStates() const { return m_hvpmt_setStates; }

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

        std::mutex m_mtx;

        std::atomic<State> m_state;

        std::thread thread_handle_HV;
        std::thread thread_handle_TDC;

        std::vector<int> m_hvpmt_setValues;
        std::vector<int> m_hvpmt_setStates;
};
