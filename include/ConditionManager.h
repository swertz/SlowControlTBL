#pragma once

#include <mutex>
#include <atomic>
#include <thread>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>

#include "SetupManager.h"
#include "Interface.h"
#include "RealSetupManager.h"
#include "FakeSetupManager.h"
#include "VmeUsbBridge.h"

class Interface;

class ConditionManager {
    
    public:

        ConditionManager(Interface& m_interface):
            m_interface(m_interface),
            m_state(State::idle),
            m_hvpmt({
                    { 1025, 0, 0, false, true, false },
                    { 925, 0, 0, false, true, false },
                    { 1225, 0, 0, false, true, false },
                    { 0, 0, 0, false, false, false }
                    })
        {
            std::cout << "Checking if the PC is connected to board..." << std::endl;
            UsbController *dummy_controller = new UsbController(DEBUG);
            bool canTalkToBoards = (dummy_controller->getStatus() == 0);
            std::cout << "Deleting dummy USB controller..." << std::endl;
            delete dummy_controller;
            if (canTalkToBoards) {
                std::cout << "You are on 'the' machine connected to the boards and can take action on them." << std::endl;
                m_setup_manager = std::make_shared<RealSetupManager>(m_interface);
            } else {
                std::cout << "WARNING : You are not on 'the' machine connected to the boards. Actions on the setup will be ignored." << std::endl;
                m_setup_manager = std::make_shared<FakeSetupManager>(m_interface);
            }

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
         * Aggregate for an HV card:
         * Set/actual HV value and card state
         */
        struct HVPMT {
            int setValue;
            int readValue;
            int readCurrent;
            bool valueChanged;
            bool setState;
            //int readState;
            bool stateChanged;
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
        
        std::mutex& getHVLock() { return m_hv_mtx; }
        std::mutex& getTDCLock() { return m_tdc_mtx; }

        /*
         * Define/retrieve/propagate the PMT HV conditions
         * So far, this is a vector with entry==channel
         */
        void setHVPMTValue(std::size_t id, int value) { m_hvpmt.at(id).setValue = value; }
        void setHVPMTState(std::size_t id, bool state) { m_hvpmt.at(id).setState = state; }
        bool propagateHVPMTValue(std::size_t id);
        bool propagateHVPMTState(std::size_t id);
        int getHVPMTSetValue(std::size_t id) const { return m_hvpmt.at(id).setValue; }
        int getHVPMTReadValue(std::size_t id) const { return m_hvpmt.at(id).readValue; }
        int getHVPMTReadCurrent(std::size_t id) const { return m_hvpmt.at(id).readCurrent; }
        bool getHVPMTSetState(std::size_t id) const { return m_hvpmt.at(id).setState; }
        //int getHVPMTReadState(std::size_t id) const { return m_hvpmt.at(id).readState; }
        std::size_t getNHVPMT() const { return m_hvpmt.size(); }

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

        std::mutex m_hv_mtx;
        std::mutex m_tdc_mtx;

        Interface& m_interface;
        std::atomic<State> m_state;

        std::thread thread_handle_HV;
        std::thread thread_handle_TDC;

        std::vector<HVPMT> m_hvpmt;
        
        std::shared_ptr<SetupManager> m_setup_manager;
};
