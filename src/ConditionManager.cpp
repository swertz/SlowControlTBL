#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <cstddef>

#include "ConditionManager.h"
#include "Interface.h"

#include "VmeUsbBridge.h"
#include "HV.h"

// Static
const std::vector< std::pair<ConditionManager::State, ConditionManager::State> > ConditionManager::m_transitions = {
    { ConditionManager::State::idle, ConditionManager::State::configured },
    { ConditionManager::State::configured, ConditionManager::State::running },
    { ConditionManager::State::running, ConditionManager::State::idle },
    { ConditionManager::State::configured, ConditionManager::State::idle }
};

// Static
std::string ConditionManager::stateToString(ConditionManager::State state) {
    switch(state) {
        case State::idle:
            return "idle";
        case State::configured:
            return "configured";
        case State::running:
            return "running";
        default:
            return "Unknown state!";
    }
}

// Static
bool ConditionManager::checkTransition(ConditionManager::State state_from, ConditionManager::State state_to) { 
    return std::find(m_transitions.begin(), m_transitions.end(), std::pair<State, State>( { state_from, state_to } ) ) != m_transitions.end();
}

bool ConditionManager::setState(ConditionManager::State state) {
    if( state == m_state ) {
        std::cout << "ConditionManager is already in state " << stateToString(m_state) << std::endl;
        return false;
    }
   
    if( !checkTransition(m_state, state) ) { 
        throw std::runtime_error("Transition from " + stateToString(m_state) + " to " + stateToString(state) + " is not allowed.");
    }
    
    std::cout << "Changing ConditionManager state from " << stateToString(m_state) << " to " << stateToString(state) << std::endl;

    m_state = state;

    return true;
}

bool ConditionManager::propagateHVPMTValue(std::size_t id) {
    bool result = m_setup_manager->setHVPMT(id);

    return result;
}

bool ConditionManager::propagateHVPMTState(std::size_t id) {
    if (getHVPMTSetState(id))
        return m_setup_manager->switchHVPMTON(id);
    else
        return m_setup_manager->switchHVPMTOFF(id);
}

bool ConditionManager::propagateDiscriSettings() {
    return m_setup_manager->propagateDiscriSettings();
}


void ConditionManager::startDaemons() {
    if( setState(State::running) ) {
        thread_handle_HV = std::thread(&ConditionManager::daemonHV, std::ref(*this));
        thread_handle_TDC = std::thread(&ConditionManager::daemonTDC, std::ref(*this));
    }
}

void ConditionManager::stopDaemons() {
    if( setState(State::idle) ) {
        thread_handle_HV.join();
        thread_handle_TDC.join();
    }
}

void ConditionManager::daemonHV() {
    while(m_state == State::running) {
        // wait some time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      
        std::lock_guard<std::mutex> m_lock(m_hv_mtx);
        std::vector< std::pair<double, double> > hv_values = m_setup_manager->getHVPMTValue();
        for (std::size_t id = 0; id < hv_values.size(); id++) {
            //m_hvpmt.at(id).readState = m_hvpmt.at(id).readState;
            m_hvpmt.at(id).readValue = hv_values.at(id).first;
            m_hvpmt.at(id).readCurrent = hv_values.at(id).second;
        }

    }
}

void ConditionManager::daemonTDC() {
    while(m_state == State::running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
