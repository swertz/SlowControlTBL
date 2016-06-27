#include <iostream>
#include <string>
#include <chrono>
#include <atomic>
#include <exception>

#include <json/writer.h>

#include "LoggingManager.h"
#include "ConditionManager.h"
#include "Interface.h"
#include "Utils.h"

LoggingManager::LoggingManager(Interface& m_interface, uint32_t m_continuous_log_time):
    m_interface(m_interface),
    m_conditions(m_interface.getConditions()),
    is_running(true),
    m_continuous_log_time(m_continuous_log_time),
    m_condition_json_list(Json::arrayValue)
{
    std::cout << "Creating LoggingManager." << std::endl;
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
    m_continuous_log = std::make_shared<CSV>("cont_log.csv");

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
    
    Json::Value this_condition;
    Json::Value hv_values;

    // Lock the conditions manager to read all the values at once
    std::lock_guard<std::mutex> hv_lock(m_conditions.getHVLock());
    
    for (size_t id = 0; id < m_conditions.getNHVPMT(); id++) {
        hv_values[ "hv_" + std::to_string(id) + "_setValue" ] = m_conditions.getHVPMTSetValue(id);
        hv_values[ "hv_" + std::to_string(id) + "_readValue" ] = m_conditions.getHVPMTReadValue(id);
        hv_values[ "hv_" + std::to_string(id) + "_setState" ] = m_conditions.getHVPMTSetState(id);
    }
    
    this_condition["hv_values"] = hv_values;
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
    file_stream.open("cond_log.json"); // FIXME add run number
    if(!file_stream.is_open())
        throw std::ios_base::failure("Could not open file cond_log.json");
    file_stream << m_writer.write(m_condition_json_root);
    file_stream.close();
}    
