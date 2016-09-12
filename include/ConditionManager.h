#pragma once

#include <mutex>
#include <thread>
#include <atomic>
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
#include "Event.h"

class Interface;

class ConditionManager {
    
    public:

        ConditionManager(Interface& m_interface):
            m_interface(m_interface),
            m_HV_daemon_running(false),
            m_hvpmt({
                    { 1025, 0, 0, true },
                    { 925, 0, 0, true },
                    { 1225, 0, 0, true },
                    { 0, 0, 0, false }
                    }),
            m_discriChannels({
                    { true, 5, 200 },
                    { true, 5, 200 },
                    { true, 5, 200 },
                    { false, 5, 200 },
                    { false, 5, 200 }
                    }),
            m_channelsMajority(3),
            m_triggerChannel(1),
            m_triggerRandomFrequency(0),
            m_TDC_backPressuring(false),
            m_TDC_fatal(false),
            m_TDC_evtCounter(0),
            m_TDC_evtBuffer_flushSize(50)
        {
            // No reliable way of knowing how many events we have
            // in the TDC buffer if there are more than 1000
            if (m_TDC_evtBuffer_flushSize > 1000)
                m_TDC_evtBuffer_flushSize = 1000;

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

            startHVDaemon();
        }

        ~ConditionManager() {
            try { 
                stopTDCReading();
            } catch(daemon_state_error) {};
            
            try { 
                stopHVDaemon();
            } catch(daemon_state_error) {};
        }

        class daemon_state_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        /*
         * Aggregate for an HV card:
         * Set/actual HV value and card state
         */
        struct HVPMT {
            int setValue;
            int readValue;
            int readCurrent;
            bool setState;
            //int readState;
        };

        // Discri settings
        struct DiscriChannel {
            bool included;
            int threshold;
            int width;
        };

        /* 
         * Get locks. The locks are NOT locked in getters/setters below, because we assume users
         * will take care of that when they call them. 
         * Trying to lock a mutex in a getter/setter while the same lock was locked before by the
         * user would result in freezing the program.
         */
        std::mutex& getHVLock() { return m_hv_mtx; }
        std::mutex& getTDCLock() { return m_tdc_mtx; }
        std::mutex& getTTCLock() { return m_ttc_mtx; }
        std::mutex& getDiscriLock() { return m_discri_mtx; }

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

        // Trigger control: channel 1 --> physics,  channel 5 --> random
        void setTriggerChannel(int channel) { m_triggerChannel = channel; }
        int getTriggerChannel() { return m_triggerChannel; }
        void setTriggerRandomFrequency(int frequency) { m_triggerRandomFrequency = frequency; }
        int getTriggerRandomFrequency() { return m_triggerRandomFrequency; }
        void startTrigger();
        void stopTrigger();

        /*
         * Define/retrieve/propagate the Discriminator conditions
         */
        void setDiscriChannelState(std::size_t id, bool state) { m_discriChannels.at(id).included = state; }
        bool getDiscriChannelState(std::size_t id) const { return m_discriChannels.at(id).included; }
        void setDiscriChannelThreshold(std::size_t id, int threshold) { m_discriChannels.at(id).threshold = threshold; }
        int getDiscriChannelThreshold(std::size_t id) const { return m_discriChannels.at(id).threshold; }
        void setDiscriChannelWidth(std::size_t id, int width) { m_discriChannels.at(id).width = width; }
        int getDiscriChannelWidth(std::size_t id) const { return m_discriChannels.at(id).width; }
        std::size_t getNDiscriChannels() const { return m_discriChannels.size(); }
        int getChannelsMajority() const { return m_channelsMajority; }
        void setChannelsMajority(int majority) { m_channelsMajority = majority; }
        bool propagateDiscriSettings();


        /*
         * Start/stop the TDC reading daemon
         */
        void startTDCReading();
        void stopTDCReading();
        std::vector<event>& getTDCEventBuffer() { return m_TDC_evtBuffer; };
        int64_t getTDCEventCount() { return m_TDC_evtCounter; }

        /*
         * Daemons: will run as threads in the background,
         * handle the HV & TDC cards
         */
        void daemonHV();
        void daemonTDC();

        void startHVDaemon();
        void stopHVDaemon();
       
    private:

        std::mutex m_hv_mtx;
        std::mutex m_discri_mtx;
        std::mutex m_ttc_mtx;
        std::mutex m_tdc_mtx;

        Interface& m_interface;

        std::thread thread_handle_HV;
        std::atomic<bool> m_HV_daemon_running;
        std::thread thread_handle_TDC;

        std::vector<HVPMT> m_hvpmt;
        std::vector<DiscriChannel> m_discriChannels;
        int m_channelsMajority;

        int m_triggerChannel;
        int m_triggerRandomFrequency;

        std::vector<event> m_TDC_evtBuffer;
        bool m_TDC_backPressuring;
        bool m_TDC_fatal;
        int64_t m_TDC_evtCounter;
        std::size_t m_TDC_evtBuffer_flushSize;
        
        std::shared_ptr<SetupManager> m_setup_manager;
};
