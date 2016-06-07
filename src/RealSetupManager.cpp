#include <cstddef>
#include <iostream>

#include "VmeUsbBridge.h"
#include "HV.h"

#include "RealSetupManager.h"
#include "Interface.h"
#include "ConditionManager.h"

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

bool RealSetupManager::setHVPMT(std::size_t id) {
    std::cout << "Setting the HV PMT..." << std::endl;
    
    int set_hv_value = m_interface.getConditions().getHVPMTSetValue(id);
    return m_hvpmt.setChV(set_hv_value, id) == 1;
}

bool RealSetupManager::switchHVPMTON(std::size_t id) {
    std::cout << "Switching the HV PMT ON..." << std::endl;
    
    return m_hvpmt.setChState(1, id) == 1;
}

bool RealSetupManager::switchHVPMTOFF(std::size_t id) {
    std::cout << "Switching the HV PMT OFF..." << std::endl;
    
    return m_hvpmt.setChState(0, id) == 1;
}
