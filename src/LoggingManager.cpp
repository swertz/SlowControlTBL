#include <iostream>
#include <string>
#include <chrono>
#include <atomic>
#include <ctime>
#include <exception>

#include <jsoncpp/json/writer.h>

#include "LoggingManager.h"
#include "Interface.h"

LoggingManager::LoggingManager(Interface& m_interface): 
    m_interface(m_interface),
    m_setup(m_interface.getSetup()),
    is_running(true)
{
    std::cout << "Creating LoggingManager." << std::endl;
    
    m_condition_log["start_time"] = timeNowString<m_clock>();

    Json::Value conditions;
    Json::Value hv_values;
    hv_values["hv_0"] = m_setup.getHV();
    
    conditions["hv_values"] = hv_values;
    m_condition_log["conditions"] = conditions;

    m_continuous_log.open("cont_log.csv");
    if(!m_continuous_log.is_open())
        throw std::ios_base::failure("Could not open file cont_log.csv");
    m_continuous_log << "timestamp,hv_0" << std::endl;
}

void LoggingManager::run(){
    const int logging_time = 1000;
    auto logging_start = timeNow<m_clock>();
    
    const int interface_time = 250;
    auto interface_start = timeNow<m_clock>();
    
    const int run_time = 20;
    auto run_start = timeNow<m_clock>();

    while(is_running){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        auto interface_stop = timeNow<m_clock>();
        std::chrono::duration<double> interface_delta = interface_stop - interface_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(interface_delta).count() >= interface_time){
            interface_start = interface_stop;
            
            m_interface.notifyUpdate();
        }  
        
        auto run_stop = timeNow<m_clock>();
        std::chrono::duration<double> run_delta = run_stop - run_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(run_delta).count() >= run_time){
            run_start = run_stop;
            
            std::lock_guard<std::mutex> lock(m_setup.getLock());
            int counter = m_setup.getCounter();
            counter++;
            m_setup.setCounter(counter);
        }
        
        auto logging_stop = timeNow<m_clock>();
        std::chrono::duration<double> logging_delta = logging_stop - logging_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(logging_delta).count() >= logging_time){
            logging_start = logging_stop;
            
            int counter = m_setup.getCounter();
            int hv = m_setup.getHV();
            m_continuous_log << timeToString<m_clock>(logging_stop) << "," << hv << std::endl;
            std::cout << "Logging: " << counter << " -- HV = " << hv << " -- " << timeToString<m_clock>(logging_stop) << std::endl;
        }
    }

    std::cout << "Stopping logger." << std::endl;
    finalize();
}

void LoggingManager::finalize() {
    m_condition_log["stop_time"] = timeNowString<m_clock>();

    Json::StyledWriter m_writer;
    std::cout << m_writer.write(m_condition_log) << std::endl;

    std::ofstream file_stream;
    file_stream.open("cond_log.json");
    if(!file_stream.is_open())
        throw std::ios_base::failure("Could not open file cond_log.json");
    file_stream << m_writer.write(m_condition_log);
    file_stream.close();
}    

void LoggingManager::stop() {
    std::cout << "LoggingManager was asked to stop." << std::endl;
    is_running = false;
}

