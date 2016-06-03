#include <cstddef>
#include <iostream>

#include "VmeUsbBridge.h"
#include "HV.h"

#include "RealSetupManager.h"
#include "Interface.h"

RealSetupManager::RealSetupManager(Interface& m_interface):
    m_interface(m_interface),
    m_controller(UsbController(NORMAL)),
    m_hvpmt(hv(&m_controller, 0xF0000, 2)) 
    { }

RealSetupManager::~RealSetupManager() {
    for (std::size_t id = 0; id < m_interface.getConditions().getNHVPMT(); id++) {
        m_hvpmt.setChState(0, id);
    }
}

void RealSetupManager::setHVPMT(std::size_t id) {
    std::cout << "Setting the HV PMT..." << std::endl;
    
    int set_hv_value = m_interface.getConditions().getHVPMTSetValue(id);
    m_hvpmt.setChV(set_hv_value, id);
    std::cout << "  HV " << id << " set to " << set_hv_value << " volts." << std::endl;
}

void RealSetupManager::switchHVPMTON(std::size_t id) {
    std::cout << "Switching the HV PMT ON..." << std::endl;
    
    m_hvpmt.setChState(1, id);
    std::cout << "  HV " << id << " switched to state 1." <<  std::endl;
}

void RealSetupManager::switchHVPMTOFF(std::size_t id) {
    std::cout << "Switching the HV PMT OFF..." << std::endl;
    
    m_hvpmt.setChState(0, id);
    std::cout << "  HV " << id << " switched to state 0." <<  std::endl;
}
