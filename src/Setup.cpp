#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>

#include "Setup.h"

// Static
const std::vector< std::pair<Setup::State, Setup::State> > Setup::m_transitions = {
    { Setup::State::idle, Setup::State::configured },
    { Setup::State::configured, Setup::State::running },
    { Setup::State::running, Setup::State::idle },
    { Setup::State::configured, Setup::State::idle }
};

// Static
std::string Setup::stateToString(Setup::State state) {
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
bool Setup::checkTransition(Setup::State state_from, Setup::State state_to) { 
    return std::find(m_transitions.begin(), m_transitions.end(), std::pair<State, State>( { state_from, state_to } ) ) != m_transitions.end();
}

bool Setup::setState(Setup::State state) {
    if( state == m_state ) {
        std::cout << "Setup is already in state " << stateToString(m_state) << std::endl;
        return false;
    }
   
    if( !checkTransition(m_state, state) ) { 
        throw std::runtime_error("Transition from " + stateToString(m_state) + " to " + stateToString(state) + " is not allowed.");
    }
    
    std::cout << "Changing Setup state from " << stateToString(m_state) << " to " << stateToString(state) << std::endl;

    m_state = state;

    return true;
}

void Setup::startDaemons() {
    if( setState(State::running) ) {
        thread_handle_HV = std::thread(&Setup::daemonHV, std::ref(*this));
        thread_handle_TDC = std::thread(&Setup::daemonTDC, std::ref(*this));
    }
}

void Setup::stopDaemons() {
    if( setState(State::idle) ) {
        thread_handle_HV.join();
        thread_handle_TDC.join();
    }
}

void Setup::daemonHV() {
    while(m_state == State::running) {
        // wait some time
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // - request reading: get promise from card
        // - await response: read promise from card
    }
}

void Setup::daemonTDC() {
    while(m_state == State::running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
