#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>

#include "Interface.h"
#include "LoggingManager.h"
#include "ConditionManager.h"
#include "HVGroup.h"

Interface::Interface(QWidget *parent): 
    QWidget(parent),
    m_conditions(new ConditionManager(*this)),
    running(false) {

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
        connect(quit, &QPushButton::clicked, qApp, &QApplication::quit);

        resize(500, 200);
        //showFullScreen();
    }

ConditionManager& Interface::getConditions() {
    return *m_conditions;
}

void Interface::notifyUpdate() {
}

void Interface::updateConditionLog() {
    if (running)
        m_logging_manager->updateConditionManagerLog();
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
