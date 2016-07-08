#pragma once

#include <cstddef>
#include <vector>

#include "SetupManager.h"

class Interface;

class FakeSetupManager: public SetupManager {
    
    public:

        FakeSetupManager(Interface& m_interface);

        virtual ~FakeSetupManager() override {};

        virtual bool setHVPMT(std::size_t id) override;
        virtual bool switchHVPMTON(std::size_t id) override;
        virtual bool switchHVPMTOFF(std::size_t id) override;
        virtual std::vector< std::pair<double, double> > getHVPMTValue() override;

        virtual bool propagateDiscriSettings() override;

    private:

        Interface& m_interface;
};
