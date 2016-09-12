#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <cstddef>

#include "ConditionManager.h"
#include "Interface.h"

#include "VmeUsbBridge.h"
#include "Event.h"

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
    if (thread_handle_HV.joinable()) {
        throw daemon_state_error("HV daemon was already running");
    }
    m_HV_daemon_running = true;
    thread_handle_HV = std::thread(&ConditionManager::daemonHV, std::ref(*this));
}

void ConditionManager::stopHVDaemon() {
    if (!thread_handle_HV.joinable()) {
        throw daemon_state_error("HV daemon was not running");
    }
    m_HV_daemon_running = false;
    thread_handle_HV.join();
}

void ConditionManager::daemonHV() {
    while (m_HV_daemon_running) {
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
    /*if (m_interface.getState() != Interface::State::configured) {
        throw daemon_state_error("TDC daemon can only be started by a configured interface");
    }*/
    
    if (thread_handle_TDC.joinable()) {
        throw daemon_state_error("TDC daemon was already running");
    }
    
    std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
    m_TDC_evtCounter = 0;
    m_TDC_evtBuffer.clear();
    m_TDC_backPressuring = false;
    m_TDC_fatal = false;

    thread_handle_TDC = std::thread(&ConditionManager::daemonTDC, std::ref(*this));
}

void ConditionManager::stopTDCReading() {
    /*if (m_interface.getState() != Interface::State::running) {
        throw daemon_state_error("TDC daemon can only be stopped by a running interface");
    }*/
    
    if (!thread_handle_TDC.joinable()) {
        throw daemon_state_error("TDC daemon was not running");
    }

    thread_handle_TDC.join();
    
    std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
    
    m_TDC_evtCounter = 0;
    m_TDC_evtBuffer.clear();
    m_TDC_backPressuring = false;
    m_TDC_fatal = false;
}

void ConditionManager::daemonTDC() {
    
    while(m_interface.getState() == Interface::State::running) {
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
