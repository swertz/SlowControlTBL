#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTimer>

#include "Interface.h"
#include "LoggingManager.h"
#include "ConditionManager.h"
#include "HVGroup.h"
#include "DiscriSettingsWindow.h"

Interface::Interface(QWidget *parent): 
    QWidget(parent),
    m_conditions(new ConditionManager(*this)),
    m_discriTunerBtn(new QPushButton("Discriminator Settings")),
    running(false) {

        std::cout << "Creating Interface. Qt version: " << qVersion() << "." << std::endl;
        QGridLayout *master_grid = new QGridLayout();

        QGroupBox *run_box = new QGroupBox("Run control");
        QVBoxLayout *run_layout = new QVBoxLayout();
        
        QPushButton *startBtn = new QPushButton("Start");
        QPushButton *stopBtn = new QPushButton("Stop");
        
        m_hv_group = new HVGroup(*this);

        run_layout->addWidget(startBtn);
        run_layout->addWidget(stopBtn);
        //run_layout->addWidget(m_discriTunerBtn);
        run_box->setLayout(run_layout);
        
        QPushButton *quit = new QPushButton("Quit");

        master_grid->addWidget(run_box, 0, 0);
        master_grid->addWidget(m_hv_group, 0, 1);
        // FIXME position and button dispaching 
        master_grid->addWidget(m_discriTunerBtn, 1, 0);
        master_grid->addWidget(quit, 2, 0);

        setLayout(master_grid);

        connect(startBtn, &QPushButton::clicked, this, &Interface::startLoggingManager);
        connect(stopBtn, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(m_discriTunerBtn, &QPushButton::clicked, this, &Interface::showDiscriSettingsWindow);
        connect(quit, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(quit, &QPushButton::clicked, qApp, &QApplication::quit);

        resize(1000, 300);
        
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &Interface::notifyUpdate);
        m_timer->start(250);
    }

ConditionManager& Interface::getConditions() {
    return *m_conditions;
}

void Interface::notifyUpdate() {
    m_hv_group->notifyUpdate();
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

// When clicking the "DiscriSetting button", open a pop up window
// and disable the button to prevent opening dozens of windows
void Interface::showDiscriSettingsWindow() {
    //this->hide();
    DiscriSettingsWindow *discriSettingsWindow = new DiscriSettingsWindow(*this);
    discriSettingsWindow->setWindowTitle("Discriminator Settings Dialog Box");
    discriSettingsWindow->show();
    m_discriTunerBtn->setDisabled(1);
    m_discriTunerBtn->setText("Discriminator Settings (already open)");
}
