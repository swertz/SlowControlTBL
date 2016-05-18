#include "RealSetupManager.h"
#include "Interface.h"

RealSetupManager::RealSetupManager(Interface& m_interface):
    m_interface(m_interface),
    m_controller(UsbController(NORMAL)),
    m_hvpmt(hv(&m_controller, 0xF0000, 2)) 
    { }

void RealSetupManager::setHVPMT() {
    std::vector <int> hvpmt_setValues = m_interface.getConditions().getHVPMTSetValues();
    std::cout << "Setting the HV PMT..." << std::endl;
    for (int entry = 0; entry < hvpmt_setValues.size(); entry++){
        m_hvpmt.setChV(hvpmt_setValues[entry], entry);
        std::cout << "  HV " << entry << " set to " << hvpmt_setValues[entry] << " volts." << std::endl;
    }
    std::cout << "Done!" << std::endl;
}

void RealSetupManager::switchHVPMTON() {
    std::cout << "Switching the HV PMT ON..." << std::endl;
    std::vector <int> hvpmt_state = m_interface.getConditions().getHVPMTSetStates();
    for (int entry = 0; entry < hvpmt_state.size(); entry++){
        m_hvpmt.setChState(hvpmt_state[entry], entry);
        std::cout << "  HV " << entry << " switched to state " << hvpmt_state[entry] << "." <<  std::endl;
    }
    std::cout << "Done!" << std::endl;
}

void RealSetupManager::switchHVPMTOFF() {
    std::cout << "Switching the HV PMT OFF..." << std::endl;
    std::vector <int> hvpmt_state = m_interface.getConditions().getHVPMTSetStates();
    for (int entry = 0; entry < hvpmt_state.size(); entry++){
        m_hvpmt.setChState(0, entry);
        std::cout << "  HV " << entry << " switched to state 0." <<  std::endl;
    }
    std::cout << "Done!" << std::endl;
}


