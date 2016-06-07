#include <QString>

#include <memory>
#include <mutex>
#include <cstddef>

#include "HVGroup.h"
#include "Interface.h"
#include "ConditionManager.h"

HVGroup::HVGroup(Interface& m_interface):
    m_interface(m_interface),
    QWidget(&m_interface) {

        m_box = new QGroupBox("HV Settings", this);

        m_layout = new QGridLayout();

        for (int hv_id = 0; hv_id < m_interface.m_conditions->getNHVPMT(); hv_id++) {
            int setHVValue = m_interface.m_conditions->getHVPMTSetValue(hv_id);
            
            HVEntry hventry;
            
            const QString label = "HV " + QString::number(hv_id) + " value:";
            hventry.label = new QLabel(label);
            
            hventry.spin_box = new QSpinBox();
            hventry.spin_box->setRange(0, 2000);
            hventry.spin_box->setWrapping(1);
            hventry.spin_box->setSingleStep(1);
            hventry.spin_box->setValue(setHVValue);
            
            hventry.value_label = new QLabel(QString::number(hventry.spin_box->value()));
            hventry.value_label->hide();
            
            //connect(hventry.spin_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &HVGroup::valueChanged);
            
            m_hventries.push_back(hventry);
            
            m_layout->addWidget(hventry.label, hv_id, 0);
            m_layout->addWidget(hventry.spin_box, hv_id, 1);
            m_layout->addWidget(hventry.value_label, hv_id, 1);
        }

        m_on_btn = new QPushButton("Switch On");
        //connect(m_on_btn, &QPushButton::clicked, [&](){ m_interface.getSetupManager()->switchHVPMTON(); });
        connect(m_on_btn, &QPushButton::clicked, this, &HVGroup::switchON);
        connect(m_on_btn, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        m_layout->addWidget(m_on_btn, m_interface.m_conditions->getNHVPMT(), 0);

        m_off_btn = new QPushButton("Switch OFF");
        connect(m_off_btn, &QPushButton::clicked, this, &HVGroup::switchOFF);
        connect(m_off_btn, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        m_layout->addWidget(m_off_btn, m_interface.m_conditions->getNHVPMT(), 0);
        m_off_btn->hide();

        m_set = new QPushButton("Set");
        connect(m_set, &QPushButton::clicked, this, &HVGroup::setHV);
        connect(m_set, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        m_layout->addWidget(m_set, m_interface.m_conditions->getNHVPMT(), 1);
        m_box->setLayout(m_layout);

}

//void HVGroup::valueChanged(int m_hv) {
//    if(m_interface.isRunning()) {
//        //m_spin_hv0->setValue(m_interface.getConditions().getHVPMTSetValues()[0]);
//        return;
//    }
//}

void HVGroup::notifyUpdate() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    if (m_interface.m_conditions->getHVPMTReadState(0)) {
        m_on_btn->hide();
        m_off_btn->show();
    } else {
        m_off_btn->hide();
        m_on_btn->show();
    }
}

void HVGroup::switchON() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    // Update Condition manager with new HV states
    for (std::size_t id = 0; id < m_hventries.size(); id++) {
        m_interface.m_conditions->setHVPMTState(id, 1);
    }
}

void HVGroup::switchOFF() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    // Update Condition manager with new HV states
    for (std::size_t id = 0; id < m_hventries.size(); id++) {
        m_interface.m_conditions->setHVPMTState(id, 0);
    }
}

void HVGroup::setHV() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    // Update Condition manager with new HV set values
    for (std::size_t id = 0; id < m_hventries.size(); id++) {
        HVEntry hventry = m_hventries.at(id);
        int new_value = m_interface.m_conditions->setHVPMTValue(id, hventry.spin_box->value());
        hventry.value_label->setText(QString::number(new_value));
    }
}

void HVGroup::setRunning() {
    for (HVEntry hventry: m_hventries) {
        hventry.spin_box->hide();
        hventry.value_label->show();
    }
    m_set->setDisabled(1);
}

void HVGroup::setNotRunning() {
    for (HVEntry hventry: m_hventries) {
        hventry.spin_box->show();
        hventry.value_label->hide();
    }
    m_set->setDisabled(0);
}
