#ifndef SETUPMANAGER_H
#define SETUPMANAGER_H

#include <vector>

class SetupManager {
    public:
        virtual void setHVPMT() = 0;
        virtual void switchHVPMTON() = 0;
        virtual void switchHVPMTOFF() = 0;
};
#endif
