#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <atomic>
#include <exception>

#include <json/writer.h>

#include "LoggingManager.h"
#include "ConditionManager.h"
#include "Interface.h"
#include "Utils.h"
#include "PythonDB.h"

LoggingManager::LoggingManager(Interface& m_interface, std::uint32_t run_number, std::string m_path, std::uint32_t m_continuous_log_time):
    m_interface(m_interface),
    m_run_number(run_number),
    m_log_path(m_path),
    m_conditions(m_interface.getConditions()),
    is_running(true),
    m_continuous_log_time(m_continuous_log_time),
    m_condition_json_list(Json::arrayValue),
    m_TDC_eventBuffer_flushSize(500)
{
    std::cout << "Creating LoggingManager for run number " << run_number << "." << std::endl;

    // Try to connect to OpenTSDB client
    try {
        m_DB = std::make_shared<OpenTSDBInterface>();
    } catch (OpenTSDBInterface::initialise_error& e) {
        std::cerr << "Warning when starting LoggingManager: " << e.what() << std::endl;
        m_DB.reset();
    }
    
    initConditionManagerLog();
    initContinuousLog();
}

LoggingManager::~LoggingManager() {
    std::cout << "Destroying LoggingManager." << std::endl;
    
    finalizeConditionManagerLog();
    finalizeContinuousLog();
}

bool LoggingManager::checkRunNumber(std::uint32_t number, std::string log_path) {
    if (std::ifstream(log_path + "/cont_log_run_" + std::to_string(number) + ".csv")) {
        return true;
    }
    if (std::ifstream(log_path + "/cond_log_run_" + std::to_string(number) + ".json")) {
        return true;
    }
    if (std::ifstream(log_path + "/events_run_" + std::to_string(number) + ".root")) {
        return true;
    }
    return false;
}

void LoggingManager::run(){
    std::cout << "Starting logger." << std::endl;
    
    auto logging_start = m_clock::now();

    // Fill the log once at start
    updateContinuousLog(logging_start);
    
    while(is_running){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
       
        // Loop updating the continuous log every X
        auto logging_stop = m_clock::now();
        std::chrono::duration<double> logging_delta = logging_stop - logging_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(logging_delta).count() >= m_continuous_log_time){
            logging_start = logging_stop;
            updateContinuousLog(logging_stop);
        }
    }

    std::cout << "Stopping logger." << std::endl;
}

void LoggingManager::stop() {
    is_running = false;
}


//--- Continuous logging

void LoggingManager::initContinuousLog() {
    // Create all the time series for values to be monitored in-browser
    if (m_DB.get()) {
        for (std::size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
            m_timeSeries_HVPMT_readVal.push_back(
                    m_DB->addTimeSeries(
                        "HVPMT.read", 
                        { 
                            { "hv", std::to_string(id) },
                            { "run_number", std::to_string(m_run_number) }
                        }
                    )
                );
            m_timeSeries_HVPMT_setVal.push_back(
                    m_DB->addTimeSeries(
                        "HVPMT.set", 
                        { 
                            { "hv", std::to_string(id) },
                            { "run_number", std::to_string(m_run_number) }
                        }
                    )
                );
        }

        m_timeSeries_TDC_interfaceEventBufferCounter = m_DB->addTimeSeries("TDC.nIntEvtBuffer", { { "run_number", std::to_string(m_run_number) } });
        m_timeSeries_TDC_FIFOEventBufferCounter = m_DB->addTimeSeries("TDC.nFIFOEvtBuffer", { { "run_number", std::to_string(m_run_number) } });
        m_timeSeries_TDC_eventCounter = m_DB->addTimeSeries("TDC.nEvt", { { "run_number", std::to_string(m_run_number) } });
        m_timeSeries_TDC_offset = m_DB->addTimeSeries("TDC.offset", { { "run_number", std::to_string(m_run_number) } });
        m_timeSeries_TTC_eventCounter = m_DB->addTimeSeries("TTC.nEvt", { { "run_number", std::to_string(m_run_number) } });
    
        for (const auto& reading: ConditionManager::ScalerReadings)
            m_timeSeries_scaler[reading.first] = m_DB->addTimeSeries("Scaler." + reading.second.first,  { { "run_number", std::to_string(m_run_number) } });
    }

    // Initialise the CSV file
    m_continuous_log = std::make_shared<CSV>(m_log_path + "/cont_log_run_" + std::to_string(m_run_number) + ".csv");

    m_continuous_log->addField("timestamp");
    for (std::size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
        m_continuous_log->addField("hv_" + std::to_string(id) + "_setValue");
        m_continuous_log->addField("hv_" + std::to_string(id) + "_readValue");
    }
    m_continuous_log->addField("tdc_nEvt");
    m_continuous_log->addField("tdc_offset");
    m_continuous_log->addField("ttc_nEvt");
    for (const auto& reading: ConditionManager::ScalerReadings)
        m_continuous_log->addField(reading.second.first);
    
    m_continuous_log->freeze();

    // Initialise the ROOT file
    std::string root_file_name = m_log_path + "/events_run_" + std::to_string(m_run_number) + ".root";
    m_root_file = new TFile(root_file_name.c_str(), "recreate");
    m_tree = new TTree("Events", "Events");
    m_tree->Branch("Event", &m_tmp_event);
}
    
void LoggingManager::updateContinuousLog(m_clock::time_point log_time, bool last_time) {
    std::uint64_t time_now = timeNowStamp<m_clock>(log_time);
    
    m_continuous_log->setField("timestamp", time_now);
    
    // Fill HV-related information
    for (std::size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
        std::lock_guard<std::mutex> hv_lock(m_conditions.getHVLock());
        
        m_continuous_log->setField("hv_" + std::to_string(id) + "_setValue", m_conditions.getHVPMTSetValue(id));
        m_continuous_log->setField("hv_" + std::to_string(id) + "_readValue", m_conditions.getHVPMTReadValue(id));
        
        if (m_DB.get()) {
            m_DB->putValue(m_timeSeries_HVPMT_setVal.at(id), m_conditions.getHVPMTSetValue(id), time_now);
            m_DB->putValue(m_timeSeries_HVPMT_readVal.at(id), m_conditions.getHVPMTReadValue(id), time_now);
        }
    }

    // Fill TDC-related information
    {
        std::lock_guard<std::mutex> tdc_lock(m_conditions.getTDCLock());

        if (m_conditions.getTDCEventBuffer().size() >= m_TDC_eventBuffer_flushSize || last_time) {
            for (const auto& e: m_conditions.getTDCEventBuffer()) {
                m_tmp_event = e;
                m_tree->Fill();
            }
            m_conditions.getTDCEventBuffer().clear();
        }
 
        m_continuous_log->setField("tdc_nEvt", m_conditions.getTDCEventCount());
        m_continuous_log->setField("tdc_offset", m_conditions.getTDCOffset());
        
        if (m_DB.get()) {
            m_DB->putValue(m_timeSeries_TDC_interfaceEventBufferCounter, m_conditions.getTDCEventBuffer().size(), time_now);
            m_DB->putValue(m_timeSeries_TDC_FIFOEventBufferCounter, m_conditions.getTDCFIFOEventCount(), time_now);
            m_DB->putValue(m_timeSeries_TDC_eventCounter, m_conditions.getTDCEventCount(), time_now);
            m_DB->putValue(m_timeSeries_TDC_offset, m_conditions.getTDCOffset(), time_now);
        }
    }

    // Fill Trigger-related information
    {
        std::lock_guard<std::mutex> tcc_lock(m_conditions.getTTCLock());
        
        std::uint64_t ttc_evt_count = m_conditions.getTriggerEventNumber();
        
        m_continuous_log->setField("ttc_nEvt", ttc_evt_count);
        
        if (m_DB.get()) {
            m_DB->putValue(m_timeSeries_TTC_eventCounter, ttc_evt_count, time_now);
        }
    }
    
    // Fill Scaler-related information
    {
        std::lock_guard<std::mutex> scaler_lock(m_conditions.getScalerLock());
        
        for (const auto& reading: ConditionManager::ScalerReadings) {
            double rate = m_conditions.getScalerRate(reading.first);
            m_continuous_log->setField(reading.second.first, rate);
        
            if (m_DB.get()) {
                m_DB->putValue(m_timeSeries_scaler.at(reading.first), rate, time_now);
            }
        }
    }
 
 
    m_continuous_log->putLine();
}

void LoggingManager::finalizeContinuousLog() {
    // Update continuous log one last time
    updateContinuousLog(m_clock::now(), true);
    
    m_continuous_log.reset();

    m_tree->Write();
    m_root_file->Close();
    m_tree = NULL;
    m_root_file = NULL;
}

//--- ConditionManager logging

void LoggingManager::initConditionManagerLog() {
    auto start_time = m_clock::now();
    m_condition_json_root["start_time_human"] = timeToString<m_clock>(start_time);
    m_condition_json_root["start_time"] = timeToJson<m_clock>(start_time); 
    updateConditionManagerLog(true, start_time);
}

void LoggingManager::updateConditionManagerLog(bool first_time, m_clock::time_point log_time) {
    std::cout << "Updating conditions log" << std::endl;

    m_condition_json_root["conditions_changed"] = !first_time;
    m_condition_json_root["run_number"] = m_run_number; 
    
    Json::Value this_condition;
    Json::Value hv_values;
    Json::Value discri_values;

    { // Lock the HV using the conditions manager to read all the values at once
        std::lock_guard<std::mutex> hv_lock(m_conditions.getHVLock());
        
        for (std::size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
            Json::Value this_value;
            this_value["setValue"] = m_conditions.getHVPMTSetValue(id);
            this_value["readValue"] = m_conditions.getHVPMTReadValue(id);
            this_value["setState"] = m_conditions.getHVPMTSetState(id);
            hv_values[ "hv_" + std::to_string(id) ] = this_value;
        }
    } // End HV lock 
    
    { // Lock the Discriminator using the conditions manager to read all the values at once
        std::lock_guard<std::mutex> discri_lock(m_conditions.getDiscriLock());

        for (std::size_t id = 0; id < m_conditions.getNDiscriChannels(); id++) {
            Json::Value this_value;
            this_value["included"] = m_conditions.getDiscriChannelState(id);
            this_value["threshold"] = m_conditions.getDiscriChannelThreshold(id);
            this_value["width"] = m_conditions.getDiscriChannelWidth(id);
            discri_values[ "discri_" + std::to_string(id) ] = this_value;
            discri_values[ "discri_majority" ] = m_conditions.getChannelsMajority();
        }
    } // End Discriminator lock 
    
    this_condition["hv_values"] = hv_values;
    this_condition["discri_values"] = discri_values;
    this_condition["time"] = timeToJson<m_clock>(log_time); 
    
    m_condition_json_list.append(this_condition);
}

void LoggingManager::finalizeConditionManagerLog() {
    auto stop_time = m_clock::now();
    m_condition_json_root["stop_time_human"] = timeToString<m_clock>(stop_time);
    m_condition_json_root["stop_time"] = timeToJson<m_clock>(stop_time); 
    m_condition_json_root["conditions"] = m_condition_json_list;

    Json::StyledWriter m_writer;

    std::ofstream file_stream;
    file_stream.open(m_log_path + "/cond_log_run_" + std::to_string(m_run_number) + ".json");
    if(!file_stream.is_open())
        throw std::ios_base::failure("Could not open file cond_log.json");
    file_stream << m_writer.write(m_condition_json_root);
    file_stream.close();
}    
