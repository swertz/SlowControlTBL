#include <QString>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include <mutex>

#include "Trigger_TDC_Group.h"
#include "Interface.h"
#include "ConditionManager.h"

Trigger_TDC_Group::Trigger_TDC_Group(Interface& m_interface):
    m_interface(m_interface),
    QGroupBox("Trigger + TDC", &m_interface) {

        QVBoxLayout *tdc_ttc_layout = new QVBoxLayout();
        
        QHBoxLayout *triggerControl_layout = new QHBoxLayout();
        QLabel *triggerLabel = new QLabel("Trigger control: ");
        QLabel *triggerChannelLabel = new QLabel("Channel");
        m_triggerChannel_box = new QSpinBox();
        m_triggerChannel_box->setRange(-1, 7);
        m_triggerChannel_box->setValue(m_interface.m_conditions->getTriggerChannel());
        QLabel *triggerRandomLabel = new QLabel("Random Freq mode");
        m_triggerRandom_box = new QSpinBox();
        m_triggerRandom_box->setRange(0, 7);
        m_triggerRandom_box->setValue(m_interface.m_conditions->getTriggerRandomFrequency());
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
        setLayout(tdc_ttc_layout);
}

void Trigger_TDC_Group::notifyUpdate() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getTDCLock());
    
    if (m_interface.m_conditions->checkTDCBackPressure()) {
        m_tdc_backPressure_label->show();
        m_tdc_ok_label->hide();
    } else {
        m_tdc_backPressure_label->hide();
    }
    
    if (m_interface.m_conditions->checkTDCFatalError()) {
        m_tdc_fatal_label->show();
        m_tdc_ok_label->hide();
    } else {
        m_tdc_fatal_label->hide();
    }
    
    if (!m_interface.m_conditions->checkTDCFatalError() && !m_interface.m_conditions->checkTDCBackPressure()) {
        m_tdc_ok_label->show();
    }
    
    m_tdc_eventCounter_label->setText("TDC events recorded: " + QString::number(m_interface.m_conditions->getTDCEventCount()));
}

void Trigger_TDC_Group::atConfigureRun() {
    if (m_interface.m_state != Interface::State::idle)
        return;

    // Freeze trigger configuration
    m_triggerChannel_box->setDisabled(true);
    m_triggerRandom_box->setDisabled(true);
    
    {
        // Propagate trigger info to condition manager and setup
        std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getTTCLock());
        m_interface.m_conditions->setTriggerChannel(m_triggerChannel_box->value());
        m_interface.m_conditions->setTriggerRandomFrequency(m_triggerRandom_box->value());
    }

    {
        std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getTDCLock());
        m_interface.m_conditions->configureTDC();
    }
    
    m_tdc_eventCounter_label->show();
}

void Trigger_TDC_Group::atStartRun() {
    if (m_interface.m_state != Interface::State::configured)
        return;

    m_interface.m_conditions->startTDCReading();

    // Start trigger
    {
        std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getTTCLock());
        m_interface.m_conditions->startTrigger();
    }
 
    m_tdc_ok_label->show();
}

void Trigger_TDC_Group::atStopRun() {
    if (m_interface.m_state == Interface::State::idle)
        return;

    if (m_interface.m_state == Interface::State::running) {
        // Stop listening for TDC events
        m_interface.m_conditions->stopTDCReading();
        
        // Stop trigger
        {
            std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getTTCLock());
            m_interface.m_conditions->stopTrigger();
        }
    }

    m_triggerChannel_box->setDisabled(false);
    m_triggerRandom_box->setDisabled(false);

    m_tdc_eventCounter_label->hide();
    m_tdc_eventCounter_label->setText("TDC events recorded: 0");
    m_tdc_backPressure_label->hide();
    m_tdc_fatal_label->hide();
    m_tdc_ok_label->hide();
}
