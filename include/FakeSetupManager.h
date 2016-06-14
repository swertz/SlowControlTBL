#pragma once

#include <cstddef>

#include "SetupManager.h"

class FakeSetupManager: public SetupManager {
    
    public:

        virtual ~FakeSetupManager() override {};

        virtual bool setHVPMT(std::size_t id) override;
        virtual bool switchHVPMTON(std::size_t id) override;
        virtual bool switchHVPMTOFF(std::size_t id) override;
};
