#pragma once

#include <cstddef>
#include <vector>

#include "SetupManager.h"

#include "VmeUsbBridge.h"
#include "HV.h"
#include "Discri.h"
#include "TTCvi.h"
#include "TDC.h"
#include "Event.h"

class Interface;

class RealSetupManager: public SetupManager {
    public:

        RealSetupManager(Interface& m_interface);
        
        /*
         * Destructor: turn the HV off
         */
        virtual ~RealSetupManager() override;

        virtual bool setHVPMT(std::size_t id) override;
        virtual bool switchHVPMTON(std::size_t id) override;
        virtual bool switchHVPMTOFF(std::size_t id) override;
        virtual std::vector< std::pair<double, double> > getHVPMTValue() override;
        //virtual int getHVPMTState(size_t id) override;

        // Trigger control : channel 1 --> physics trigger, channel 5 --> random triggers
        virtual void setTrigger(int channel, int randomFrequency) override;

        // Discriminator/coincidence manager
        virtual bool propagateDiscriSettings() override;

        // TDC settings
        virtual void setTDCWindowOffset(int offset) override;
        virtual void setTDCWindowWidth(int width) override;
        virtual unsigned int getTDCStatus() override;
        virtual int getTDCNEvents() override;
        virtual event getTDCEvent() override;

    private:

        UsbController m_controller;
        hv m_hvpmt;
        discri m_discri;
        ttcVi m_TTC;
        tdc m_TDC;
        
        Interface& m_interface;
};
