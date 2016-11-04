#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>

#include "SetupManager.h"
#include "Interface.h"
#include "RealSetupManager.h"
#include "FakeSetupManager.h"
#include "Utils.h"

#include "Event.h"

class Interface;

class ConditionManager {
    
    public:

        ConditionManager(Interface& m_interface);
        ~ConditionManager();

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
         * ALL functions requesting a lock are documented => ALWAYS check the functions you are calling
         * if you request a lock yourself!!!
         */
        std::mutex& getHVLock() { return m_hv_mtx; }
        std::mutex& getTDCLock() { return m_tdc_mtx; }
        std::mutex& getTTCLock() { return m_ttc_mtx; }
        std::mutex& getDiscriLock() { return m_discri_mtx; }
        std::mutex& getScalerLock() { return m_scaler_mtx; }

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
        void resetTrigger();
        std::uint64_t getTriggerEventNumber();

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
         * Public, since done by interface when starting/stopping run
         */
        void startTDCReading();
        void stopTDCReading();
        /*
         * Configure the TDC
         */
        void configureTDC();
        std::vector<event>& getTDCEventBuffer() { return m_TDC_evtBuffer; };
        std::int64_t getTDCEventCount() { return m_TDC_evtCounter; }
        std::int64_t getTDCFIFOEventCount();
        bool checkTDCBackPressure() { return m_TDC_backPressuring; }
        bool checkTDCFatalError() { return m_TDC_fatal; }
        std::size_t getTDCOffset() { return m_TDC_offsetMinimum(); }

        // Defined in .cpp: list of rate measurements using the scaler
        // Maps a channel ID to a pair with a string (name of the measurement) and a double (constant multiplying the rate)
        static const std::map<ScalerChannel, std::pair<std::string, double>> ScalerReadings;
        double getScalerRate(ScalerChannel channel) { return m_scaler_rates.at(channel)(); }
        void resetScaler() { m_setup_manager->resetScaler(); }

    private:

        /* 
         * HV daemon: updates read values for voltage & current
         * LOCKS: HV
         */
        void daemonHV();
        /* 
         * TDC daemon: reads TDC events, backpressures trigger if needed
         * LOCKS: TDC, TTC
         */
        void daemonTDC();
        /* 
         * Scaler daemon: reads Scaler at fixed time intervals, computes rates
         * LOCKS: Scaler
         */
        void daemonScaler();

        /*
         * Start and stop HV daemon as background thread
         * Private, since done when ConditionManager is constructed/destroyed
         */
        void startHVDaemon();
        void stopHVDaemon();
       
        /*
         * Start/stop the Scaler reading daemon
         * Private, since done when ConditionManager is constructed/destroyed
         */
        void startScalerDaemon();
        void stopScalerDaemon();

        std::mutex m_hv_mtx;
        std::mutex m_discri_mtx;
        std::mutex m_ttc_mtx;
        std::mutex m_tdc_mtx;
        std::mutex m_scaler_mtx;

        Interface& m_interface;

        std::thread thread_handle_HV;
        std::atomic<bool> m_HV_daemon_running;
        std::thread thread_handle_TDC;
        std::atomic<bool> m_TDC_daemon_running;
        std::thread thread_handle_scaler;
        std::atomic<bool> m_scaler_daemon_running;

        std::vector<HVPMT> m_hvpmt;
        std::vector<DiscriChannel> m_discriChannels;
        int m_channelsMajority;

        int m_triggerChannel;
        int m_triggerRandomFrequency;

        std::vector<event> m_TDC_evtBuffer;
        MovingMinimum<std::size_t> m_TDC_offsetMinimum;
        std::atomic<bool> m_TDC_backPressuring;
        std::atomic<bool> m_TDC_fatal;
        std::int64_t m_TDC_evtCounter;
        std::size_t m_TDC_evtBuffer_flushSize;

        uint64_t m_scaler_interval;
        std::map<ScalerChannel, Rate<std::chrono::high_resolution_clock>> m_scaler_rates;
        
        std::shared_ptr<SetupManager> m_setup_manager;
};
