#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "SetupManager.h"
#include "Event.h"

class Interface;

class FakeSetupManager: public SetupManager {
    
    public:

        FakeSetupManager(Interface& m_interface);

        virtual ~FakeSetupManager() override {};

        virtual bool setHVPMT(std::size_t id) override;
        virtual bool switchHVPMTON(std::size_t id) override;
        virtual bool switchHVPMTOFF(std::size_t id) override;
        virtual std::vector< std::pair<double, double> > getHVPMTValue() override;

        virtual void setTrigger(int channel, int randomFrequency) override;
        virtual void resetTrigger() override;
        // Return 10 (same as getTDCNEvents() to stay "in sync")
        virtual std::int64_t getTTCEventNumber() override;

        virtual bool propagateDiscriSettings() override;
        
        virtual void setTDCWindowOffset(int offset) override;
        virtual void setTDCWindowWidth(int width) override;
        // Return status for data ready
        virtual unsigned int getTDCStatus() override;
        // Return 10
        virtual int getTDCNEvents() override;
        // Return an empty, but valid, event
        virtual event getTDCEvent() override;
        virtual void configureTDC() override;

    private:

        Interface& m_interface;
};
