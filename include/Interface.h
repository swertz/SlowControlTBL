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

class Interface : public QWidget {
    
    Q_OBJECT

    public:
        Interface(QWidget *parent = 0);
        virtual ~Interface() {}

        Setup& getSetup();

        void notifyUpdate();

    private slots:
        void startLoggingManager();
        void stopLoggingManager();
        void onPlus();
        void onMinus();

    private:
      
        void setCounter(int i);

        QLabel *label;
        std::shared_ptr<LoggingManager> m_logging_manager;
        std::shared_ptr<Setup> m_setup;
        std::thread thread_handler;
};
