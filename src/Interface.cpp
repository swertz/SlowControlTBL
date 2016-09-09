#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QSpinBox>
#include <QMessageBox>

#include <limits>

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

        /* ----- Run control box ----- */
        QGroupBox *run_box = new QGroupBox("Run control");
        QVBoxLayout *run_layout = new QVBoxLayout();
       
        QHBoxLayout *runNumber_layout = new QHBoxLayout();
        QLabel *runLabel = new QLabel("Run number: ");
        runLabel->setAlignment(Qt::AlignCenter);
        m_runNumberLabel = new QLabel();
        m_runNumberLabel->setAlignment(Qt::AlignCenter);
        m_runNumberSpin = new QSpinBox();
        m_runNumberSpin->setRange(0, std::numeric_limits<int>::max());
        runNumber_layout->addWidget(runLabel);
        runNumber_layout->addWidget(m_runNumberLabel);
        runNumber_layout->addWidget(m_runNumberSpin);

        QHBoxLayout *triggerControl_layout = new QHBoxLayout();
        QLabel *triggerLabel = new QLabel("Trigger control: ");
        QLabel *triggerChannelLabel = new QLabel("Channel");
        m_triggerChannel_box = new QSpinBox();
        m_triggerChannel_box->setRange(-1, 7);
        m_triggerChannel_box->setValue(m_conditions->getTriggerChannel());
        QLabel *triggerRandomLabel = new QLabel("Random Freq mode");
        m_triggerRandom_box = new QSpinBox();
        m_triggerRandom_box->setRange(0, 7);
        m_triggerRandom_box->setValue(m_conditions->getTriggerRandomFrequency());
        triggerControl_layout->addWidget(triggerLabel);
        triggerControl_layout->addWidget(triggerChannelLabel);
        triggerControl_layout->addWidget(m_triggerChannel_box);
        triggerControl_layout->addWidget(triggerRandomLabel);
        triggerControl_layout->addWidget(m_triggerRandom_box);
        
        QPushButton *startBtn = new QPushButton("Start");
        QPushButton *stopBtn = new QPushButton("Stop");
        
        run_layout->addWidget(startBtn);
        run_layout->addWidget(stopBtn);
        run_layout->addLayout(runNumber_layout);
        run_layout->addLayout(triggerControl_layout);
        run_box->setLayout(run_layout);
        
        /* ----- HV control box ----- */
        m_hv_group = new HVGroup(*this);
        
        /* ----- Master grid ----- */
        QPushButton *quit = new QPushButton("Quit");

        QGridLayout *master_grid = new QGridLayout();
        
        master_grid->addWidget(run_box, 0, 0);
        master_grid->addWidget(m_hv_group, 0, 1);
        master_grid->addWidget(m_discriTunerBtn, 1, 0);
        master_grid->addWidget(quit, 2, 0);

        setLayout(master_grid);

        /* ----- Connect signals ----- */
        connect(startBtn, &QPushButton::clicked, this, &Interface::startRun);
        connect(stopBtn, &QPushButton::clicked, this, &Interface::stopRun);
        connect(m_discriTunerBtn, &QPushButton::clicked, this, &Interface::showDiscriSettingsWindow);
        connect(quit, &QPushButton::clicked, this, &Interface::stopRun);
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

void Interface::startRun() {
    if (m_logging_manager.get()) {
        // We are running! Nothing to do...
        return;
    }

    uint32_t run_number = m_runNumberSpin->value();

    // Check if we can start with the given run number
    if (LoggingManager::checkRunNumber(run_number)) {
        QMessageBox msgBox(QMessageBox::Warning, "Warning", "Warning: log files for run number " + QString::number(run_number) + " already exist. Do you want to go on and overwrite them?", QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;
    }

    m_logging_manager = std::make_shared<LoggingManager>(*this, run_number);
    thread_handler = std::thread(&LoggingManager::run, std::ref(*m_logging_manager));

    // Start listening for TDC events
    m_conditions->startTDCReading();

    // Freeze trigger configuration
    m_triggerChannel_box->setDisabled(1);
    m_triggerRandom_box->setDisabled(1);
    {
        // Propagate trigger info to condition manager and setup
        std::lock_guard<std::mutex> m_lock(m_conditions->getTTCLock());
        m_conditions->setTriggerChannel(m_triggerChannel_box->value());
        m_conditions->setTriggerRandomFrequency(m_triggerRandom_box->value());
        m_conditions->startTrigger();
    }
    
    m_hv_group->setRunning();
    
    m_runNumberSpin->hide();
    m_runNumberLabel->setText(QString::number(run_number));
    m_runNumberLabel->show();

    running = true;
}
    
void Interface::stopRun() {
    if (!m_logging_manager.get()) {
        // We're not running... Nothing to stop!
        return;
    }

    // Stop listening for TDC events
    m_conditions->stopTDCReading();

    m_logging_manager->stop();
    thread_handler.join();
    m_logging_manager.reset();
    
    // Stop trigger
    {
        std::lock_guard<std::mutex> m_lock(m_conditions->getTTCLock());
        m_conditions->stopTrigger();
    }
    // Re enable the trigger settings
    m_triggerChannel_box->setDisabled(0);
    m_triggerRandom_box->setDisabled(0);

    m_hv_group->setNotRunning();
    
    m_runNumberLabel->hide();
    m_runNumberSpin->setValue(m_runNumberSpin->value() + 1);
    m_runNumberSpin->show();

    running = false;
    
    notifyUpdate();
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
