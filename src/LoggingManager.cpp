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

LoggingManager::LoggingManager(Interface& m_interface, uint32_t run_number, uint32_t m_continuous_log_time):
    m_interface(m_interface),
    m_run_number(run_number),
    m_conditions(m_interface.getConditions()),
    is_running(true),
    m_continuous_log_time(m_continuous_log_time),
    m_condition_json_list(Json::arrayValue)
{
    std::cout << "Creating LoggingManager for run number " << run_number << "." << std::endl;
}

bool LoggingManager::checkRunNumber(uint32_t number) {
    if (std::ifstream("cont_log_run_" + std::to_string(number) + ".csv")) {
        return true;
    }
    if (std::ifstream("cond_log_run_" + std::to_string(number) + ".json")) {
        return true;
    }
    return false;
}

void LoggingManager::run(){
    
    std::cout << "Starting logger." << std::endl;
    
    initConditionManagerLog();
    initContinuousLog();
    
    auto logging_start = m_clock::now();
    
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
    finalizeConditionManagerLog();
    finalizeContinuousLog();
}

void LoggingManager::stop() {
    std::cout << "LoggingManager was asked to stop." << std::endl;
    is_running = false;
}


//--- Continuous logging

void LoggingManager::initContinuousLog() {
    m_continuous_log = std::make_shared<CSV>("cont_log_run_" + std::to_string(m_run_number) + ".csv");

    m_continuous_log->addField("timestamp");
    
    for (size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
        m_continuous_log->addField("hv_" + std::to_string(id) + "_setValue");
        m_continuous_log->addField("hv_" + std::to_string(id) + "_readValue");
    }
    
    m_continuous_log->freeze();
}
    
void LoggingManager::updateContinuousLog(m_clock::time_point log_time) {
    // Lock the conditions manager to read all the values at once
    std::lock_guard<std::mutex> hv_lock(m_conditions.getHVLock());
    
    m_continuous_log->setField("timestamp", log_time.time_since_epoch().count());
    
    for (size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
        m_continuous_log->setField("hv_" + std::to_string(id) + "_setValue", m_conditions.getHVPMTSetValue(id));
        m_continuous_log->setField("hv_" + std::to_string(id) + "_readValue", m_conditions.getHVPMTReadValue(id));
    }

    m_continuous_log->putLine();
}

void LoggingManager::finalizeContinuousLog() {
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
        
        for (size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
            Json::Value this_value;
            this_value["setValue"] = m_conditions.getHVPMTSetValue(id);
            this_value["readValue"] = m_conditions.getHVPMTReadValue(id);
            this_value["setState"] = m_conditions.getHVPMTSetState(id);
            hv_values[ "hv_" + std::to_string(id) ] = this_value;
        }
    } // End HV lock 
    
    { // Lock the Discriminator using the conditions manager to read all the values at once
        std::lock_guard<std::mutex> discri_lock(m_conditions.getDiscriLock());

        for (size_t id = 0; id < m_conditions.getNDiscriChannels(); id++) {
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
    file_stream.open("cond_log_run_" + std::to_string(m_run_number) + ".json");
    if(!file_stream.is_open())
        throw std::ios_base::failure("Could not open file cond_log.json");
    file_stream << m_writer.write(m_condition_json_root);
    file_stream.close();
}    
