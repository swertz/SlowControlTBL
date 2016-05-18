#ifndef FAKESETUPMANAGER_H
#define FAKESETUPMANAGER_H

#include "SetupManager.h"

class FakeSetupManager : public SetupManager {
    
    public:

        virtual void setHVPMT() override;
        virtual void switchHVPMTON() override;
        virtual void switchHVPMTOFF() override;
};
#endif
