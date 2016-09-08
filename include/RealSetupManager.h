#pragma once

#include <cstddef>
#include <vector>

#include "SetupManager.h"

#include "VmeUsbBridge.h"
#include "HV.h"
#include "Discri.h"
#include "TTCvi.h"

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

        // Trigger control : channel 1 --> physics trigger, channel 5 --> random triggers
        virtual void setTrigger(int channel, int randomFrequency) override;

        // Discriminator/coincidence manager
        virtual bool propagateDiscriSettings() override;

    private:

        UsbController m_controller;
        hv m_hvpmt;
        discri m_discri;
        ttcVi m_TTC;
        
        Interface& m_interface;
};
