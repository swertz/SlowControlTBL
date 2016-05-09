#pragma once

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

#include <thread>
#include <memory>

#include "LoggingManager.h"
#include "Setup.h"
#include "HVGroup.h"

class Interface : public QWidget {
    friend class HVGroup;
    
    Q_OBJECT

    public:
        Interface(QWidget *parent = 0);
        virtual ~Interface() {}

        Setup& getSetup();

        void notifyUpdate();

        bool isRunning() const { return running; }

    private slots:
        void startLoggingManager();
        void stopLoggingManager();
        void onPlus();
        void onMinus();
        void setHV(int hv_value);

    private:
      
        void setCounter(int i);

        QLabel *label;
        HVGroup* m_hv_group;
        std::shared_ptr<LoggingManager> m_logging_manager;
        std::shared_ptr<Setup> m_setup;
        std::thread thread_handler;

        bool running;
};
