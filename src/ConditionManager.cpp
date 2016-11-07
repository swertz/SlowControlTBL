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
const std::map<ScalerChannel, std::pair<std::string, double>> ConditionManager::ScalerReadings = {
    { ScalerChannel::PM0, { "PM0", 1 } },
    { ScalerChannel::PM1, { "PM1", 1 } },
    { ScalerChannel::NIM, { "NIM", 1 } },
    { ScalerChannel::VME, { "VME", 1 } },
    { ScalerChannel::TTC, { "TTC", 1 } },
    { ScalerChannel::Ileak, { "Ileak", 1e-3 } }
};


ConditionManager::ConditionManager(Interface& m_interface, bool use_fake_setup):
    m_interface(m_interface),
    m_HV_daemon_running(false),
    m_TDC_daemon_running(false),
    m_hvpmt({
            { 1350, 0, 0, true },
            { 1350, 0, 0, true },
            }),
    m_discriChannels({
            { true, 30, 200 },
            { true, 30, 200 },
            { false, 30, 200 },
            { false, 30, 200 },
            { false, 30, 200 }
            }),
    m_channelsMajority(2),
    m_triggerChannel(1),
    m_triggerRandomFrequency(0),
    m_TDC_offsetMinimum(5),
    m_TDC_backPressuring(false),
    m_TDC_fatal(false),
    m_TDC_evtCounter(0),
    m_TDC_evtBuffer_flushSize(50),
    m_scaler_interval(5000)
{
    // No reliable way of knowing how many events we have
    // in the TDC buffer if there are more than 1000
    if (m_TDC_evtBuffer_flushSize > 1000)
        m_TDC_evtBuffer_flushSize = 1000;

    bool canTalkToBoards = false;
    if (!use_fake_setup) {
        std::cout << "Checking if the PC is connected to board..." << std::endl;
        UsbController *dummy_controller = new UsbController(DEBUG);
        canTalkToBoards = (dummy_controller->getStatus() == 0);
        std::cout << "Deleting dummy USB controller..." << std::endl;
        delete dummy_controller;
    }
    if (canTalkToBoards) {
        std::cout << "You are on 'the' machine connected to the boards and can take action on them." << std::endl;
        m_setup_manager = std::make_shared<RealSetupManager>(m_interface);
    } else {
        std::cout << "WARNING : You are not on 'the' machine connected to the boards. Actions on the setup will be ignored." << std::endl;
        m_setup_manager = std::make_shared<FakeSetupManager>(m_interface);
    }

    for (const auto& reading: ScalerReadings)
        m_scaler_rates[reading.first] = Rate<std::chrono::high_resolution_clock>(reading.second.second);

    startHVDaemon();
}

ConditionManager::~ConditionManager() {
    try { 
        stopTDCReading();
    } catch(daemon_state_error) {};
    
    try { 
        stopHVDaemon();
    } catch(daemon_state_error) {};
    
    try { 
        stopScalerDaemon();
    } catch(daemon_state_error) {};
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

void ConditionManager::resetTrigger() {
    m_setup_manager->resetTrigger();
}

std::uint64_t ConditionManager::getTriggerEventNumber() {
    return m_setup_manager->getTTCEventNumber();
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
    if (thread_handle_TDC.joinable()) {
        throw daemon_state_error("TDC daemon was already running");
    }
    
    m_TDC_daemon_running = true;
    thread_handle_TDC = std::thread(&ConditionManager::daemonTDC, std::ref(*this));
}

void ConditionManager::stopTDCReading() {
    if (!thread_handle_TDC.joinable()) {
        throw daemon_state_error("TDC daemon was not running");
    }

    m_TDC_daemon_running = false;
    thread_handle_TDC.join();
}

void ConditionManager::configureTDC() {
    m_TDC_offsetMinimum.clear();
    m_TDC_evtCounter = 0;
    m_TDC_evtBuffer.clear();
    m_TDC_backPressuring = false;
    m_TDC_fatal = false;
    
    m_setup_manager->configureTDC();
}

std::int64_t ConditionManager::getTDCFIFOEventCount() {
    std::int64_t n_evt = m_setup_manager->getTDCNEvents();
    unsigned int tdc_status = m_setup_manager->getTDCStatus();
    bool data_ready = tdc::dataReady(tdc_status);
    
    return (data_ready && n_evt == 0) ? 1000 : n_evt; 
}

void ConditionManager::daemonTDC() {

    while(m_TDC_daemon_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        unsigned int tdc_status;
        {
            std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
            tdc_status = m_setup_manager->getTDCStatus();
        }
        bool almost_full = tdc::isAlmostFull(tdc_status);
        bool lost_trigger = tdc::lostTrig(tdc_status);
        bool data_ready = tdc::dataReady(tdc_status);

        if (lost_trigger) {
            m_TDC_fatal = true;
            std::cout << "TDC fatal error: lost triggers" << std::endl;
            break;
        }

        if (almost_full) {
 
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
            
            std::size_t n_evt = 0;
 
            // First check if the number of events is high enough that it's worth
            // it to start an acquisition loop
            {
                std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
                n_evt = m_setup_manager->getTDCNEvents();
            }
            if (n_evt < m_TDC_evtBuffer_flushSize / 2) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::lock_guard<std::mutex> m_lock(m_tdc_mtx);
 
            // n_evt = 0 can happen if actual number of events between 1000 and 1024
            // Also, read at most m_TDC_evtBuffer_flushSize events at once
            if (n_evt > m_TDC_evtBuffer_flushSize || n_evt == 0)
                n_evt = m_TDC_evtBuffer_flushSize;
            
            for (std::size_t i = 0; i < n_evt; i++) {
                
                event this_evt = m_setup_manager->getTDCEvent();

                // Data is corrupt -> stop saving it!
                if (this_evt.errorCode) {
                    m_TDC_fatal = true;
                    std::cout << "TDC fatal error: event error code " << this_evt.errorCode << std::endl;
                    break;
                }
 
                // Check on the first event if TDC and TTC are in sync -> if not stop data taking!
                if (i == 0) {
                    std::int64_t evt_offset = 0;
                    {
                        std::lock_guard<std::mutex> m_ttc_lock(m_ttc_mtx);
                        // TDC buffer is a FIFO -> add number of events still in buffer
                        evt_offset = this_evt.eventNumber + m_setup_manager->getTDCNEvents() - m_setup_manager->getTTCEventNumber();
                    }
                    // Compute running minimum of offset over last X readings
                    // If offset becomes too large, stop TDC data reading
                    // Since the offset can only grow, using the running minimum is good enough
                    evt_offset = m_TDC_offsetMinimum(std::abs(evt_offset));
                    if (evt_offset > 3) {
                        m_TDC_fatal = true;
                        std::cout << "TDC fatal error: out of sync with TTC. Offset: " << evt_offset << std::endl;
                        break;
                    }
                }
                
                m_TDC_evtBuffer.push_back(this_evt);
                m_TDC_evtCounter++;
            }
        }

        if (m_TDC_fatal)
            break;
    }

}

void ConditionManager::startScalerDaemon() {
    if (thread_handle_scaler.joinable()) {
        throw daemon_state_error("Scaler daemon was already running");
    }
    m_scaler_daemon_running = true;
    thread_handle_scaler = std::thread(&ConditionManager::daemonScaler, std::ref(*this));
}

void ConditionManager::stopScalerDaemon() {
    if (!thread_handle_scaler.joinable()) {
        throw daemon_state_error("Scaler daemon was not running");
    }
    m_scaler_daemon_running = false;
    thread_handle_scaler.join();
}

void ConditionManager::daemonScaler() {

    while(m_scaler_daemon_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_scaler_interval));
        
        std::lock_guard<std::mutex> m_lock(m_scaler_mtx);

        for (const auto& reading: ScalerReadings) {
            m_scaler_rates.at(reading.first).add(m_setup_manager->getScalerCount(reading.first));
        }
    }
}

