#include <cstddef>
#include <iostream>

#include "VmeUsbBridge.h"
#include "HV.h"
#include "Discri.h"
#include "TTCvi.h"
#include "Event.h"

#include "RealSetupManager.h"
#include "Interface.h"
#include "ConditionManager.h"

RealSetupManager::RealSetupManager(Interface& m_interface):
    m_interface(m_interface),
    m_controller(UsbController(NORMAL)),
    m_hvpmt(hv(&m_controller, 0xF0000, 2)),
    m_discri(discri(&m_controller)),
    m_TTC(ttcVi(&m_controller)),
    m_TDC(&m_controller, 0x00AA0000),
    m_scaler(&m_controller)
    { }

RealSetupManager::~RealSetupManager() {
    for (std::size_t id = 0; id < m_interface.getConditions().getNHVPMT(); id++) {
        m_hvpmt.setChState(0, id);
    }
}

bool RealSetupManager::setHVPMT(std::size_t id) { 
    int set_hv_value = m_interface.getConditions().getHVPMTSetValue(id);
    std::cout << "Setting the HV PMT number " << id << " to " << set_hv_value << "." << std::endl;

    return m_hvpmt.setChV(set_hv_value, id) == 1;
}

bool RealSetupManager::switchHVPMTON(std::size_t id) {
    std::cout << "Switching the HV PMT " << id << " ON..." << std::endl;
    
    return m_hvpmt.setChState(1, id) == 1;
}

bool RealSetupManager::switchHVPMTOFF(std::size_t id) {
    std::cout << "Switching the HV PMT " << id << " OFF..." << std::endl;
    
    return m_hvpmt.setChState(0, id) == 1;
}

std::vector< std::pair<double, double> > RealSetupManager::getHVPMTValue() {
    // FIXME Maybe we could have a function reading value of only one PM
    // Would require to modify Martin's library
    std::vector< std::pair<double, double> > hv_values;
    double ** temp_values = 0;
    temp_values = m_hvpmt.readValues(temp_values);
    for (std::size_t id = 0; id < m_interface.getConditions().getNHVPMT(); id++) {
        hv_values.push_back(std::make_pair(temp_values[id][0], temp_values[id][1]));
    }
    return hv_values;
}

bool RealSetupManager::propagateDiscriSettings() {
    // Take action together with checking if the action succeded 
    // NB : Setup functions return several different int's so far
    
    bool succeeded_majority = (m_discri.setMajority(m_interface.getConditions().getChannelsMajority()) == 1);

    bool succeeded_discriSettings = true;
    for (int dc_id = 0; dc_id < m_interface.getConditions().getNDiscriChannels(); dc_id++) {
        succeeded_discriSettings = (succeeded_discriSettings && ((m_discri.setChannel(dc_id, m_interface.getConditions().getDiscriChannelState(dc_id))) == 1));
        succeeded_discriSettings = (succeeded_discriSettings && (m_discri.setTh(m_interface.getConditions().getDiscriChannelThreshold(dc_id), dc_id) == 1));
        succeeded_discriSettings = (succeeded_discriSettings && ((m_discri.setWidth(m_interface.getConditions().getDiscriChannelWidth(dc_id), dc_id)) == 1));

    }
    return succeeded_majority && succeeded_discriSettings;
}

void RealSetupManager::setTrigger(int channel, int randomFrequency) {
    if (channel == 7) {
        std::cout << "Disabling trigger..." << std::endl;
        m_TTC.changeChannel(channel);
        return;
    } else {
        std::cout << "Setting trigger to channel " << channel << "..." << std::endl;
    }
    if (channel == 5 || channel == -1) {
        std::cout << "Trigger in random mode with frequency mode " << randomFrequency << "..." << std::endl;
        m_TTC.changeRandomFrequency(randomFrequency);
    }
    m_TTC.changeChannel(channel);
}

void RealSetupManager::resetTrigger() {
    m_TTC.resetCounter();
}
        
std::int64_t RealSetupManager::getTTCEventNumber() {
    return m_TTC.getEventNumber();
}

void RealSetupManager::setTDCWindowOffset(int offset) {
    m_TDC.setWindowOffset(offset);
}

void RealSetupManager::setTDCWindowWidth(int width) {
    m_TDC.setWindowWidth(width);
}

unsigned int RealSetupManager::getTDCStatus() {
    return m_TDC.getStatusWord();
}

int RealSetupManager::getTDCNEvents() {
    return m_TDC.getNumberOfEvents();
}

event RealSetupManager::getTDCEvent() {
    return m_TDC.getEvent();
}

void RealSetupManager::configureTDC() {
    // Load all TDC parameters
    m_TDC.loadUserConfig();
    // Empty TDC buffer
    for (std::size_t i = 0; i < m_TDC.getNumberOfEvents(); i++) {
        m_TDC.getEvent();
    }
}

void RealSetupManager::resetScaler() {
    if (!m_scaler.reset())
        std::cerr << "Warning: could not reset scaler!" << std::endl;
}

int RealSetupManager::getScalerCount(ScalerChannel channel) {
    return m_scaler.getCount(static_cast<int>(channel));
}

//std::vector<double> RealSetupManager::getHVPMTState() {
//    // FIXME Not available yet in Martin's library
//    // Probably not needed as the HV value tells everything
//    std::vector<double> hv_values;
//    double ** temp_values = 0;
//    temp_values = m_hvpmt.readValues(temp_values);
//    for (std::size_t id = 0; id < m_interface.getConditions().getNHVPMT(); id++) {
//        std::cout << temp_values[id][0] << std::endl;
//        hv_values.push_back(temp_values[id][0]);
//    }
//    return hv_values;
//}
