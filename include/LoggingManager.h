#pragma once

#include <atomic>
#include <memory>
#include <fstream>

#include <json/value.h>

#include "Setup.h"

// Forward declaration
class Interface;

/*
 * Helper functions for time management
 *
 */

template<typename T>
inline std::string timeNowString() {
    std::time_t c_time = T::to_time_t(T::now());
    std::string time_str(ctime(&c_time));
    return time_str.substr(0, time_str.size() - 1);
}

template<typename T>
inline std::string timeToString(typename T::time_point m_time) {
    std::time_t c_time = T::to_time_t(m_time);
    std::string time_str(ctime(&c_time));
    return time_str.substr(0, time_str.size() - 1);
}

template<typename T>
inline typename T::time_point timeNow() {
    return T::now();
}

/*
 * LoggingManager: run by the background thread, manages all the logging: conditions (json), continuous (csv, root)
 */
class LoggingManager {
  public:

      LoggingManager(Interface& m_interface);

      void run();
      
      void stop();
  
  private:
        
      using m_clock = std::chrono::system_clock;

      void finalize();

      Interface& m_interface;
      Setup& m_setup;
      std::atomic<bool> is_running;

      Json::Value m_condition_log;

      std::ofstream m_continuous_log;
};
 
