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

// Static
const std::vector< std::pair<Interface::State, Interface::State> > Interface::m_transitions = {
    { Interface::State::idle, Interface::State::configured },
    { Interface::State::configured, Interface::State::running },
    { Interface::State::running, Interface::State::idle },
    { Interface::State::configured, Interface::State::idle }
};

// Static
std::string Interface::stateToString(Interface::State state) {
    switch(state) {
        case State::idle:
            return "idle";
        case State::configured:
            return "configured";
        case State::running:
            return "running";
        default:
            return "Unknown state!";
    }
}

// Static
bool Interface::checkTransition(Interface::State state_from, Interface::State state_to) { 
    return std::find(m_transitions.begin(), m_transitions.end(), std::pair<State, State>( { state_from, state_to } ) ) != m_transitions.end();
}

bool Interface::setState(Interface::State state) {
    if( state == m_state ) {
        std::cout << "Interface is already in state " << stateToString(m_state) << std::endl;
        return false;
    }
   
    if( !checkTransition(m_state, state) ) { 
        throw std::runtime_error("Transition from " + stateToString(m_state) + " to " + stateToString(state) + " is not allowed.");
    }
    
    std::cout << "Changing Interface state from " << stateToString(m_state) << " to " << stateToString(state) << std::endl;

    m_state = state;

    return true;
}

Interface::Interface(QWidget *parent): 
    QWidget(parent),
    m_conditions(new ConditionManager(*this)),
    m_discriTunerBtn(new QPushButton("Discriminator Settings")),
    m_state(State::idle)
    {

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

        m_configureBtn = new QPushButton("Configure");
        m_startBtn = new QPushButton("Start");
        m_startBtn->setDisabled(true);
        m_stopBtn = new QPushButton("Stop");
        m_stopBtn->setDisabled(true);
 
        run_layout->addWidget(m_configureBtn);
        run_layout->addWidget(m_startBtn);
        run_layout->addWidget(m_stopBtn);
        run_layout->addStretch();
        run_layout->addLayout(runNumber_layout);
        run_box->setLayout(run_layout);
        
        /* ----- HV control box ----- */
        m_hv_group = new HVGroup(*this);
        
        /* ------ TDC & TTC control box -----  */
        QGroupBox *tdc_ttc_box = new QGroupBox("Trigger + TDC");
        QVBoxLayout *tdc_ttc_layout = new QVBoxLayout();
        
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
        
        QHBoxLayout *tdc_status_layout = new QHBoxLayout();
        
        m_tdc_backPressure_label = new QLabel("TDC BACKPRESSURE");
        m_tdc_backPressure_label->setStyleSheet("QLabel { font-weight: bold; background-color: orange; }");
        m_tdc_backPressure_label->setAlignment(Qt::AlignCenter);
        m_tdc_backPressure_label->hide();
        tdc_status_layout->addWidget(m_tdc_backPressure_label);
        
        m_tdc_fatal_label = new QLabel("TDC FATAL ERROR");
        m_tdc_fatal_label->setStyleSheet("QLabel { font-weight: bold; background-color: red; }");
        m_tdc_fatal_label->setAlignment(Qt::AlignCenter);
        m_tdc_fatal_label->hide();
        tdc_status_layout->addWidget(m_tdc_fatal_label);
        
        m_tdc_ok_label = new QLabel("TDC taking data");
        m_tdc_ok_label->setStyleSheet("QLabel { font-weight: bold; background-color: green; }");
        m_tdc_ok_label->setAlignment(Qt::AlignCenter);
        m_tdc_ok_label->hide();
        tdc_status_layout->addWidget(m_tdc_ok_label);
        
        m_tdc_eventCounter_label = new QLabel("TDC events recorded: 0");
        m_tdc_eventCounter_label->setAlignment(Qt::AlignCenter);
        m_tdc_eventCounter_label->hide();
        tdc_status_layout->addWidget(m_tdc_eventCounter_label);

        tdc_ttc_layout->addLayout(triggerControl_layout);
        tdc_ttc_layout->addLayout(tdc_status_layout);
        tdc_ttc_box->setLayout(tdc_ttc_layout);

        /* ----- Master grid ----- */
        QPushButton *quit = new QPushButton("Quit");

        QGridLayout *master_grid = new QGridLayout();
        
        master_grid->addWidget(run_box, 0, 0);
        master_grid->addWidget(m_hv_group, 0, 1);
        master_grid->addWidget(m_discriTunerBtn, 1, 0);
        master_grid->addWidget(tdc_ttc_box, 1, 1);
        master_grid->addWidget(quit, 2, 0);

        setLayout(master_grid);

        /* ----- Connect signals ----- */
        connect(m_configureBtn, &QPushButton::clicked, this, &Interface::configureRun);
        connect(m_startBtn, &QPushButton::clicked, this, &Interface::startRun);
        connect(m_stopBtn, &QPushButton::clicked, this, &Interface::stopRun);
        connect(m_discriTunerBtn, &QPushButton::clicked, this, &Interface::showDiscriSettingsWindow);
        connect(quit, &QPushButton::clicked, this, &Interface::quit);

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &Interface::notifyUpdate);
        m_timer->start(250);
    }

ConditionManager& Interface::getConditions() {
    return *m_conditions;
}

void Interface::notifyUpdate() {
    // Update HV values
    m_hv_group->notifyUpdate();

    if (m_state == State::running) {
        // Update TDC status flags
        {
            std::lock_guard<std::mutex> m_lock(m_conditions->getTDCLock());
            if (m_conditions->checkTDCBackPressure()) {
                m_tdc_backPressure_label->show();
                m_tdc_ok_label->hide();
            } else {
                m_tdc_backPressure_label->hide();
            }
            if (m_conditions->checkTDCFatalError()) {
                m_tdc_fatal_label->show();
                m_tdc_ok_label->hide();
            } else {
                m_tdc_fatal_label->hide();
            }
            if (!m_conditions->checkTDCFatalError() && !m_conditions->checkTDCBackPressure()) {
                m_tdc_ok_label->show();
            }
            m_tdc_eventCounter_label->setText("TDC events recorded: " + QString::number(m_conditions->getTDCEventCount()));
        }
    }
}

void Interface::updateConditionLog() {
    if (m_state != State::idle)
        m_logging_manager->updateConditionManagerLog();
}

void Interface::configureRun() {
    uint32_t run_number = m_runNumberSpin->value();

    // Check if we can start with the given run number
    if (LoggingManager::checkRunNumber(run_number)) {
        QMessageBox msgBox(QMessageBox::Warning, "Warning", "Warning: log files for run number " + QString::number(run_number) + " already exist. Do you want to go on and overwrite them?", QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;
    }

    m_startBtn->setDisabled(false);
    m_stopBtn->setDisabled(false);
    m_configureBtn->setDisabled(true);

    m_logging_manager = std::make_shared<LoggingManager>(*this, run_number);
    
    // Freeze trigger configuration
    m_triggerChannel_box->setDisabled(true);
    m_triggerRandom_box->setDisabled(true);
    {
        // Propagate trigger info to condition manager and setup
        std::lock_guard<std::mutex> m_lock(m_conditions->getTTCLock());
        m_conditions->setTriggerChannel(m_triggerChannel_box->value());
        m_conditions->setTriggerRandomFrequency(m_triggerRandom_box->value());
    }

    {
        std::lock_guard<std::mutex> m_lock(m_conditions->getTDCLock());
        m_conditions->configureTDC();
    }
    
    m_runNumberSpin->hide();
    m_runNumberLabel->setText(QString::number(run_number));
    m_runNumberLabel->show();

    m_tdc_eventCounter_label->show();

    m_state = State::configured;
    
    notifyUpdate();
}

void Interface::startRun() {
    m_startBtn->setDisabled(true);
    m_stopBtn->setDisabled(false);
    m_configureBtn->setDisabled(true);

    // Start continuous logging
    thread_handler = std::thread(&LoggingManager::run, std::ref(*m_logging_manager));
    
    // Start listening for TDC events
    m_conditions->startTDCReading();

    // Start trigger
    m_conditions->startTrigger();
    
    m_tdc_ok_label->show();
    
    //m_hv_group->setRunning();
    
    m_state = State::running;
    
    notifyUpdate();
}
    
void Interface::stopRun() {
    if (m_state == State::running) {
        // Stop listening for TDC events
        m_conditions->stopTDCReading();
        
        // Stop trigger
        m_conditions->stopTrigger();

        // Stop logging
        m_logging_manager->stop();
        thread_handler.join();
 
        //m_hv_group->setNotRunning();
    }

    if (m_state == State::running || m_state == State::configured) {
        m_startBtn->setDisabled(true);
        m_stopBtn->setDisabled(true);
        m_configureBtn->setDisabled(false);
        
        // Destroy logging manager
        m_logging_manager.reset();

        // Re enable the trigger settings
        m_triggerChannel_box->setDisabled(false);
        m_triggerRandom_box->setDisabled(false);

        m_runNumberLabel->hide();
        m_runNumberSpin->setValue(m_runNumberSpin->value() + 1);
        m_runNumberSpin->show();
    
        m_tdc_eventCounter_label->hide();
        m_tdc_eventCounter_label->setText("TDC events recorded: 0");
        m_tdc_backPressure_label->hide();
        m_tdc_fatal_label->hide();
        m_tdc_ok_label->hide();
    }

    m_state = State::idle;

    notifyUpdate();
}

void Interface::quit() {
    if (m_state == State::idle) {
        qApp->quit();
    }

    if (m_state == State::configured) {
        stopRun();
        qApp->quit();
    }

    if (m_state == State::running) {
        QMessageBox msgBox(QMessageBox::Warning, "Warning", "Warning: a run is ongoing! Do you really want to quit?", QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;

        stopRun();
        qApp->quit();
    }
}

// When clicking the "DiscriSetting button", open a pop up window
// and disable the button to prevent opening dozens of windows
void Interface::showDiscriSettingsWindow() {
    DiscriSettingsWindow *discriSettingsWindow = new DiscriSettingsWindow(*this);
    discriSettingsWindow->setWindowTitle("Discriminator Settings Dialog Box");
    discriSettingsWindow->show();
    m_discriTunerBtn->setDisabled(true);
    m_discriTunerBtn->setText("Discriminator Settings (already open)");
}
