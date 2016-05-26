#pragma once

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

#include <thread>
#include <memory>

#include <unistd.h>
#include <sys/param.h>

#include "SetupManager.h"
#include "RealSetupManager.h"
#include "FakeSetupManager.h"

#include "LoggingManager.h"
#include "ConditionManager.h"
#include "HVGroup.h"


class Interface : public QWidget {
    friend class HVGroup;
    
    Q_OBJECT

    public:
        Interface(QWidget *parent = 0);
        virtual ~Interface() {}

        ConditionManager& getConditions();

        void notifyUpdate();

        bool isRunning() const { return running; }
        SetupManager* getSetupManager() const { return m_setup_manager; }

    private slots:
        void startLoggingManager();
        void stopLoggingManager();
        void onPlus();
        void onMinus();

    private:
      
        void setCounter(int i);

        QLabel* label;
        HVGroup* m_hv_group;
        std::shared_ptr<LoggingManager> m_logging_manager;
        std::shared_ptr<ConditionManager> m_conditions;
        std::thread thread_handler;

        SetupManager* m_setup_manager;

        bool running;
};
