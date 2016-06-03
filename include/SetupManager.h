#pragma once

#include <cstddef>

class SetupManager {
    public:
        virtual ~SetupManager() {};

        virtual void setHVPMT(std::size_t id) = 0;
        virtual void switchHVPMTON(std::size_t id) = 0;
        virtual void switchHVPMTOFF(std::size_t id) = 0;
};
