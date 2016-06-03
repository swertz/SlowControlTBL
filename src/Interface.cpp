#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>

#include "Interface.h"
#include "SetupManager.h"
#include "RealSetupManager.h"
#include "FakeSetupManager.h"
#include "VmeUsbBridge.h"

Interface::Interface(QWidget *parent): 
    QWidget(parent),
    m_conditions(new ConditionManager()),
    running(false) {

        std::cout << "Checking if the PC is connected to board..." << std::endl;
        UsbController *dummy_controller = new UsbController(DEBUG);
        bool canTalkToBoards = (dummy_controller->getStatus() == 0);
        std::cout << "Deleting dummy USB controller..." << std::endl;
        delete dummy_controller;
        if (canTalkToBoards) {
            std::cout << "You are on 'the' machine connected to the boards and can take action on them." << std::endl;
            m_setup_manager = std::make_shared<RealSetupManager>(*this);
        } else {
            std::cout << "WARNING : You are not on 'the' machine connected to the boards. Actions on the setup will be ignored." << std::endl;
            m_setup_manager = std::make_shared<FakeSetupManager>();
        }

        std::cout << "Creating Interface. Qt version: " << qVersion() << "." << std::endl;
        QGridLayout *master_grid = new QGridLayout();

        QGroupBox *run_box = new QGroupBox("Run control");
        QVBoxLayout *run_layout = new QVBoxLayout();
        
        QPushButton *startBtn = new QPushButton("Start");
        QPushButton *stopBtn = new QPushButton("Stop");
        QPushButton *quit = new QPushButton("Quit");
        
        m_hv_group = new HVGroup(*this);

        run_layout->addWidget(startBtn);
        run_layout->addWidget(stopBtn);
        run_layout->addWidget(quit);
        run_box->setLayout(run_layout);

        master_grid->addWidget(run_box, 0, 0);
        master_grid->addWidget(m_hv_group, 0, 1);

        setLayout(master_grid);

        connect(startBtn, &QPushButton::clicked, this, &Interface::startLoggingManager);
        connect(stopBtn, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(quit, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(quit, &QPushButton::clicked, [=](){ m_setup_manager->switchHVPMTOFF(); });
        connect(quit, &QPushButton::clicked, qApp, &QApplication::quit);

        resize(500, 200);
        //showFullScreen();
    }

ConditionManager& Interface::getConditions() {
    return *m_conditions;
}

void Interface::notifyUpdate() {
}

void Interface::startLoggingManager() {
    if(m_logging_manager.get())
        return;

    m_logging_manager = std::make_shared<LoggingManager>(*this);
    thread_handler = std::thread(&LoggingManager::run, std::ref(*m_logging_manager));

    m_hv_group->setRunning();

    running = true;
}
    
void Interface::stopLoggingManager() {
    if(!m_logging_manager.get())
        return;

    m_logging_manager->stop();
    thread_handler.join();
    m_logging_manager.reset();

    notifyUpdate();

    m_hv_group->setNotRunning();

    running = false;
}
