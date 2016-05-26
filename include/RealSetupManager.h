#pragma once

#include "SetupManager.h"
#include "VmeUsbBridge.h"
#include "HV.h"

class Interface;

class RealSetupManager : public SetupManager {
    public:

        RealSetupManager(Interface& m_interface);

        virtual void setHVPMT() override;
        virtual void switchHVPMTON() override;
        virtual void switchHVPMTOFF() override;

    private:

        UsbController m_controller;
        hv m_hvpmt;
        
        Interface& m_interface;
};
