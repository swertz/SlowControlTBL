#include <iostream>
#include <string>
#include <chrono>
#include <atomic>
#include <ctime>

#include "LoggingManager.h"
#include "Interface.h"

LoggingManager::LoggingManager(Interface& m_interface): 
    m_interface(m_interface),
    m_setup(m_interface.getSetup()),
    is_running(true) {
        std::cout << "Creating LoggingManager." << std::endl;
    }

void LoggingManager::run(){
    const int logging_time = 1000;
    std::chrono::system_clock::time_point logging_start = std::chrono::system_clock::now();
    
    const int interface_time = 250;
    std::chrono::system_clock::time_point interface_start = std::chrono::system_clock::now();
    
    const int run_time = 20;
    std::chrono::system_clock::time_point run_start = std::chrono::system_clock::now();

    while(is_running){
        std::this_thread::sleep_for(std::chrono::milliseconds(run_time/2));
        
        std::chrono::system_clock::time_point interface_stop = std::chrono::system_clock::now();
        std::chrono::duration<float> interface_delta = interface_stop - interface_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(interface_delta).count() >= interface_time){
            interface_start = interface_stop;
            
            m_interface.notifyUpdate();
        }  
        
        std::chrono::system_clock::time_point run_stop = std::chrono::system_clock::now();
        std::chrono::duration<float> run_delta = run_stop - run_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(run_delta).count() >= run_time){
            run_start = run_stop;
            
            m_setup.lock();
            int counter = m_setup.getCounter();
            counter++;
            m_setup.setCounter(counter);
            m_setup.unlock();
        }
        
        std::chrono::system_clock::time_point logging_stop = std::chrono::system_clock::now();
        std::chrono::duration<float> logging_delta = logging_stop - logging_start;
        if(std::chrono::duration_cast<std::chrono::milliseconds>(logging_delta).count() >= logging_time){
            logging_start = logging_stop;
            
            int counter = m_setup.getCounter();
            std::time_t c_time = std::chrono::system_clock::to_time_t(logging_stop);
            std::cout << "Logging: " << counter << " -- " << ctime(&c_time);
        }
    }
    
    std::cout << "Stopping logger." << std::endl;
}

void LoggingManager::stop(){
    std::cout << "LoggingManager was asked to stop." << std::endl;
    is_running = false;
}

