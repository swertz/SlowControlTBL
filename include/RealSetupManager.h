#pragma once

#include <cstddef>

#include "SetupManager.h"
#include "VmeUsbBridge.h"
#include "HV.h"

class Interface;

class RealSetupManager: public SetupManager {
    public:

        RealSetupManager(Interface& m_interface);
        
        /*
         * Destructor: turn the HV off
         */
        virtual ~RealSetupManager() override;

        virtual bool setHVPMT(size_t id) override;
        virtual bool switchHVPMTON(size_t id) override;
        virtual bool switchHVPMTOFF(size_t id) override;

    private:

        UsbController m_controller;
        hv m_hvpmt;
        
        Interface& m_interface;
};
