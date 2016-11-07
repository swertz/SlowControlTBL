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
#include <cstdint>

#include "Interface.h"
#include "LoggingManager.h"
#include "ConditionManager.h"
#include "HVGroup.h"
#include "Trigger_TDC_Group.h"
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
        m_ttc_tdc_group = new Trigger_TDC_Group(*this);

        /* ----- Discri tuner & scaler reset ----- */
        QVBoxLayout *discri_scaler_layout = new QVBoxLayout();
        m_discriTunerBtn = new QPushButton("Discriminator Settings");
        QPushButton *scaler_resetBtn = new QPushButton("Reset scaler");
        discri_scaler_layout->addWidget(m_discriTunerBtn);
        discri_scaler_layout->addWidget(scaler_resetBtn);

        /* ----- Master grid ----- */
        QPushButton *quit = new QPushButton("Quit");

        QGridLayout *master_grid = new QGridLayout();
        
        master_grid->addWidget(run_box, 0, 0);
        master_grid->addWidget(m_hv_group, 0, 1);
        master_grid->addLayout(discri_scaler_layout, 1, 0);
        master_grid->addWidget(m_ttc_tdc_group, 1, 1);
        master_grid->addWidget(quit, 2, 0);

        setLayout(master_grid);

        /* ----- Connect signals ----- */
        connect(m_configureBtn, &QPushButton::clicked, this, &Interface::configureRun);
        connect(m_startBtn, &QPushButton::clicked, this, &Interface::startRun);
        connect(m_stopBtn, &QPushButton::clicked, this, &Interface::stopRun);
        connect(m_discriTunerBtn, &QPushButton::clicked, this, &Interface::showDiscriSettingsWindow);
        connect(scaler_resetBtn, &QPushButton::clicked, this, [&](){
                    std::lock_guard<std::mutex> m_lock(m_conditions->getScalerLock());
                    m_conditions->resetScaler();
                }
            );
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
        m_ttc_tdc_group->notifyUpdate();
    }
}

void Interface::updateConditionLog() {
    if (m_state != State::idle)
        m_logging_manager->updateConditionManagerLog();
}

void Interface::configureRun() {
    std::uint32_t run_number = m_runNumberSpin->value();

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

    m_runNumberSpin->hide();
    m_runNumberLabel->setText(QString::number(run_number));
    m_runNumberLabel->show();
    
    {
        std::lock_guard<std::mutex> m_lock(m_conditions->getDiscriLock());
        m_conditions->propagateDiscriSettings();
    }

    m_logging_manager = std::make_shared<LoggingManager>(*this, run_number);
    
    m_ttc_tdc_group->atConfigureRun();

    m_state = State::configured;
    
    notifyUpdate();
}

void Interface::startRun() {
    m_startBtn->setDisabled(true);
    m_stopBtn->setDisabled(false);
    m_configureBtn->setDisabled(true);

    // Start continuous logging
    thread_handler = std::thread(&LoggingManager::run, std::ref(*m_logging_manager));
    
    m_ttc_tdc_group->atStartRun();

    //m_hv_group->setRunning();
    
    m_state = State::running;
    
    notifyUpdate();
}
    
void Interface::stopRun() {
    m_ttc_tdc_group->atStopRun();

    if (m_state == State::running) {
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

        m_runNumberLabel->hide();
        m_runNumberSpin->setValue(m_runNumberSpin->value() + 1);
        m_runNumberSpin->show();
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
