#pragma once

#include <atomic>
#include <memory>

#include "Setup.h"

class Interface;

class LoggingManager {
  public:

      LoggingManager(Interface& m_interface);

      void run();

      void stop();
  
  private:

      Interface& m_interface;
      Setup& m_setup;
      std::atomic<bool> is_running;
};
 
