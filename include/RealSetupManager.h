#pragma once

#include <cstddef>
#include <vector>

#include "SetupManager.h"

#include "VmeUsbBridge.h"
#include "HV.h"
#include "Discri.h"

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
        virtual std::vector< std::pair<double, double> > getHVPMTValue() override;
        //virtual int getHVPMTState(size_t id) override;

        // Discriminator/coincidence manager
        virtual bool propagateDiscriSettings() override;

    private:

        UsbController m_controller;
        hv m_hvpmt;
        discri m_discri;
        
        Interface& m_interface;
};
