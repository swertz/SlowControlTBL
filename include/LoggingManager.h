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
Json::UInt64 timeToJson(typename T::time_point m_time) {
    return static_cast<Json::UInt64>(m_time.time_since_epoch().count());
}

/*
 * LoggingManager: run by the background thread, manages all the logging: conditions (json), continuous (csv, root)
 */
class LoggingManager {
  public:

      LoggingManager(Interface& m_interface, uint32_t m_continuous_log_time = 1000, uint32_t m_interface_refresh_time = 250);
      ~LoggingManager() {};

      void run();
      void stop();
  
  private:
        
      using m_clock = std::chrono::system_clock;

      void initConditionLog();
      void updateConditionLog(bool first_time = false);
      void finalizeConditionLog();
      
      void initContinuousLog();
      void updateContinuousLog(m_clock::time_point log_time);
      void finalizeContinuousLog();

      Interface& m_interface;
      Setup& m_setup;
      std::atomic<bool> is_running;

      uint32_t m_continuous_log_time;
      uint32_t m_interface_refresh_time;

      Json::Value m_condition_log;
      Json::Value m_conditions;

      std::ofstream m_continuous_log;
};
 
