#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <cstddef>

#include "ConditionManager.h"
#include "Interface.h"

#include "VmeUsbBridge.h"
#include "Event.h"

// Static
const std::vector< std::pair<ConditionManager::State, ConditionManager::State> > ConditionManager::m_transitions = {
    { ConditionManager::State::idle, ConditionManager::State::configured },
    { ConditionManager::State::configured, ConditionManager::State::running },
    { ConditionManager::State::running, ConditionManager::State::idle },
    { ConditionManager::State::running, ConditionManager::State::configured },
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

void ConditionManager::startTrigger() {
    m_setup_manager->setTrigger(m_triggerChannel, m_triggerRandomFrequency);
}

void ConditionManager::stopTrigger() {
    m_setup_manager->setTrigger(7, 0); // Channel 7 means disabled...
}

void ConditionManager::startHVDaemon() {
    if (m_state == State::idle && setState(State::configured))
        thread_handle_HV = std::thread(&ConditionManager::daemonHV, std::ref(*this));
}

void ConditionManager::stopHVDaemon() {
    if (setState(State::idle))
        thread_handle_HV.join();
}

void ConditionManager::daemonHV() {
    while(m_state == State::configured || m_state == State::running) {
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

void ConditionManager::startTDCReading() {
    if (setState(State::running)) {
        {
            std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
            m_TDC_evtCounter = 0;
            m_TDC_evtBuffer.clear();
            m_TDC_backPressuring = false;
            m_TDC_fatal = false;
        }

        thread_handle_TDC = std::thread(&ConditionManager::daemonTDC, std::ref(*this));
    }
}

void ConditionManager::stopTDCReading() {
    if (m_state == State::running && setState(State::configured) )
        thread_handle_TDC.join();
    
    std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
    
    m_TDC_evtCounter = 0;
    m_TDC_evtBuffer.clear();
    m_TDC_backPressuring = false;
    m_TDC_fatal = false;
}

void ConditionManager::daemonTDC() {
    
    while(m_state == State::running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        event m_evt; 

        unsigned int tdc_status;
        {
            std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
            tdc_status = m_setup_manager->getTDCStatus();
        }
        bool almost_full = tdc::IsAlmostFull(tdc_status);
        bool lost_trigger = tdc::LostTrig(tdc_status);
        bool data_ready = tdc::DataReady(tdc_status);

        if (almost_full || lost_trigger) {
            
            // Something bad has happened or is about to happen -> backpressure the TTC
            std::lock_guard<std::mutex> m_TTC_lock(m_ttc_mtx);
            stopTrigger();
            m_TDC_backPressuring = true;
        
        } else {

            if (m_TDC_backPressuring) {
                std::lock_guard<std::mutex> m_TTC_lock(m_ttc_mtx);
                startTrigger();
                m_TDC_backPressuring = false;
            }
            
        }

        if (data_ready) {
            std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
            
            std::size_t n_evt = m_setup_manager->getTDCNEvents();
            
            // n_evt = 0 can happen if actual number of events between 1000 and 1024
            // Also, read at most m_TDC_evtBuffer_flushSize events at once
            if (n_evt > m_TDC_evtBuffer_flushSize || n_evt == 0)
                n_evt = m_TDC_evtBuffer_flushSize;

            for (std::size_t i = 0; i < n_evt; i++) {
                event this_evt = m_setup_manager->getTDCEvent();
                // FIXME
                this_evt.errorCode = 0;

                // Data is corrupt -> stop saving it!
                if (this_evt.errorCode) {
                    m_TDC_fatal = true;
                    break;
                }
                m_TDC_evtBuffer.push_back(this_evt);
                m_TDC_evtCounter++;
            }
        }


        if (m_TDC_fatal)
            break;
    }

}
