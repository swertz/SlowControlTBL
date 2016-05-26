#pragma once

#include "SetupManager.h"

class FakeSetupManager : public SetupManager {
    
    public:

        virtual void setHVPMT() override;
        virtual void switchHVPMTON() override;
        virtual void switchHVPMTOFF() override;
};
