#pragma once

#include <cstddef>

class SetupManager {
    public:
        virtual ~SetupManager() {};

        virtual bool setHVPMT(std::size_t id) = 0;
        virtual bool switchHVPMTON(std::size_t id) = 0;
        virtual bool switchHVPMTOFF(std::size_t id) = 0;
};
