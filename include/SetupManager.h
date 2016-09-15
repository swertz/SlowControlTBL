#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Event.h"

class SetupManager {
    public:
        virtual ~SetupManager() {};

        virtual bool setHVPMT(std::size_t id) = 0;
        virtual bool switchHVPMTON(std::size_t id) = 0;
        virtual bool switchHVPMTOFF(std::size_t id) = 0;
        virtual std::vector< std::pair<double, double> > getHVPMTValue() = 0;

        virtual void setTrigger(int channel, int randomFrequency) = 0;
        virtual std::int64_t getTTCEventNumber() = 0;

        virtual bool propagateDiscriSettings() = 0;
        
        virtual void setTDCWindowOffset(int offset) = 0;
        virtual void setTDCWindowWidth(int width) = 0;
        virtual unsigned int getTDCStatus() = 0;
        virtual int getTDCNEvents() = 0;
        virtual event getTDCEvent() = 0;
        virtual void configureTDC() = 0;
};
