#pragma once

#include <atomic>
#include <memory>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

#include <json/value.h>

#include "ConditionManager.h"
#include "PythonDB.h"

// Forward declaration
class Interface;

/*
 * Small class to handle CSV file writing for continuous logging
 */
class CSV {
    
    public:
        
        CSV(std::string fileName): m_frozen(false) {
            m_file.open(fileName);
            if (!m_file.is_open())
                throw std::ios_base::failure("Could not open file " + fileName);
        }
        ~CSV() { m_file.close(); }

        void freeze() {
            m_frozen = true;

            for (const auto& field: m_fields) {
                m_file << field << ",";
            }
            m_file << std::endl;
        }

        void addField(std::string field) {
            if (m_frozen) {
                std::cout << "Warning: tried to add field to frozen CSV." << std::endl;
                return;
            }

            if (std::find(m_fields.begin(), m_fields.end(), field) != m_fields.end()) {
                std::cout << "Warning: tried to add already present field in CSV." << std::endl;
                return;
            }

            m_fields.push_back(field);
        }

        template<typename T>
        void setField(std::string field, T content) {
            if (!m_frozen) {
                std::cout << "Warning: tried to set field in a non-frozen CSV." << std::endl;
                return;
            }

            if (std::find(m_fields.begin(), m_fields.end(), field) == m_fields.end()) {
                std::cout << "Warning: field " << field << " does not exist." << std::endl; 
                return;
            }

            std::ostringstream str_content;
            str_content << content;
            m_line[field] = str_content.str();
        }

        void putLine() {
            if (!m_frozen) {
                std::cout << "Warning: tried to output a non-frozen CSV." << std::endl;
                return;
            }

            for (const auto& field: m_fields) {
                m_file << m_line[field] << ",";
            }

            m_file << std::endl;

            m_line.clear();
        }

    private:

        bool m_frozen;
        std::vector<std::string> m_fields;
        std::map<std::string, std::string> m_line;
        std::ofstream m_file;
};


/*
 * LoggingManager: run by the background thread, manages all the logging: conditions (json), continuous (csv, root)
 */
class LoggingManager {
  public:
      
      using m_clock = std::chrono::system_clock;

      LoggingManager(Interface& m_interface, uint32_t run_number, uint32_t m_continuous_log_time = 1000);
      ~LoggingManager() {};

      void run();
      void stop();
      
      // Public: if interface changes something during a run, we have to update
      void updateConditionManagerLog(bool first_time = false, m_clock::time_point log_time = m_clock::now());

      /* Static: to check if creating a LoggingManager with a certain run number would overwrite existing log files
       * Return: true if log files already exist
       */
      static bool checkRunNumber(uint32_t number);
  
  private:
        
      void initConditionManagerLog();
      void finalizeConditionManagerLog();
      
      void initContinuousLog();
      void updateContinuousLog(m_clock::time_point log_time);
      void finalizeContinuousLog();

      Interface& m_interface;
      ConditionManager& m_conditions;
      std::atomic<bool> is_running;

      uint32_t m_continuous_log_time;
      uint32_t m_run_number;
      std::shared_ptr<CSV> m_continuous_log;

      std::shared_ptr<OpenTSDBInterface> m_DB;
      std::vector<std::shared_ptr<TimeSeries>> m_timeSeries_HVPMT_setVal;
      std::vector<std::shared_ptr<TimeSeries>> m_timeSeries_HVPMT_readVal;

      Json::Value m_condition_json_root;
      Json::Value m_condition_json_list;
};
 
