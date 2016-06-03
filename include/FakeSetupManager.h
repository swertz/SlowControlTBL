#pragma once

#include <cstddef>

#include "SetupManager.h"

class FakeSetupManager: public SetupManager {
    
    public:

        virtual ~FakeSetupManager() override {};

        virtual void setHVPMT(std::size_t id) override;
        virtual void switchHVPMTON(std::size_t id) override;
        virtual void switchHVPMTOFF(std::size_t id) override;
};
