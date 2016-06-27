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

        QLabel *toplabel_set_value = new QLabel("Set voltage");
        toplabel_set_value->setAlignment(Qt::AlignCenter);
        int position_set_value = 1;
        m_layout->addWidget(toplabel_set_value, 0, position_set_value);

        QLabel *toplabel_read_value = new QLabel("Read voltage");
        toplabel_read_value->setAlignment(Qt::AlignCenter);
        int position_read_value = 2;
        m_layout->addWidget(toplabel_read_value, 0, position_read_value);

        QLabel *toplabel_read_current = new QLabel("Read current");
        toplabel_read_current->setAlignment(Qt::AlignCenter);
        int position_read_current = 3;
        m_layout->addWidget(toplabel_read_current, 0, position_read_current);

        QLabel *toplabel_set_state = new QLabel("Switch ON");
        toplabel_set_state->setAlignment(Qt::AlignCenter);
        int position_set_state = 4;
        m_layout->addWidget(toplabel_set_state, 0, position_set_state);

        //m_layout->addWidget(toplabel_read_state, 0, 4);

        for (int hv_id = 0; hv_id < m_interface.m_conditions->getNHVPMT(); hv_id++) {

            int setHVValue = m_interface.m_conditions->getHVPMTSetValue(hv_id);
            
            HVEntry hventry;
            
            const QString label = "HV " + QString::number(hv_id) + " :";
            hventry.label = new QLabel(label);

            hventry.cb_set_state = new QCheckBox();
            hventry.cb_set_state->setStyleSheet("margin-left:50%; margin-right:50%;");
            hventry.cb_set_state->setChecked(m_interface.m_conditions->getHVPMTSetState(hv_id));
            //connect(hventry.cb_set_state, &QCheckBox::stateChanged, [&](){ m_interface.m_conditions->setHVPMTState(hventry.hv_number, hventry.cb_set_state->isChecked());});
            
            hventry.sb_set_value = new QSpinBox();
            hventry.sb_set_value->setRange(0, 2000);
            hventry.sb_set_value->setWrapping(1);
            hventry.sb_set_value->setSingleStep(1);
            hventry.sb_set_value->setValue(setHVValue);
            
            hventry.setValue_label = new QLabel(QString::number(hventry.sb_set_value->value()));
            hventry.setValue_label->hide();

            hventry.readValue_label = new QLabel(QString::number(m_interface.m_conditions->getHVPMTReadValue(hv_id)));
            hventry.readValue_label->setAlignment(Qt::AlignCenter);
            hventry.readValue_label->setStyleSheet("QLabel { background-color : red; }");

            hventry.readCurrent_label = new QLabel(QString::number(m_interface.m_conditions->getHVPMTReadCurrent(hv_id)));
            hventry.readCurrent_label->setAlignment(Qt::AlignCenter);
            
            m_hventries.push_back(hventry);

            const int position = hv_id+1;            
            m_layout->addWidget(hventry.label, position, 0);
            m_layout->addWidget(hventry.cb_set_state, position, position_set_state);
            m_layout->addWidget(hventry.sb_set_value, position, position_set_value);
            m_layout->addWidget(hventry.setValue_label, position, position_set_value);
            m_layout->addWidget(hventry.readValue_label, position, position_read_value);
            m_layout->addWidget(hventry.readCurrent_label, position, position_read_current);
        }

        m_on_btn = new QPushButton("Propagate");
        //connect(m_on_btn, &QPushButton::clicked, [&](){ m_interface.getSetupManager()->switchHVPMTON(); });
        connect(m_on_btn, &QPushButton::clicked, this, &HVGroup::switchON);
        connect(m_on_btn, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        m_layout->addWidget(m_on_btn, m_interface.m_conditions->getNHVPMT()+1, position_set_state);

        m_off_btn = new QPushButton("Switch OFF");
        connect(m_off_btn, &QPushButton::clicked, this, &HVGroup::switchOFF);
        connect(m_off_btn, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        m_layout->addWidget(m_off_btn, m_interface.m_conditions->getNHVPMT()+1, position_set_state);
        m_off_btn->hide();

        m_set = new QPushButton("Set");
        connect(m_set, &QPushButton::clicked, this, &HVGroup::setHV);
        connect(m_set, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        m_layout->addWidget(m_set, m_interface.m_conditions->getNHVPMT()+1, position_set_value);
        m_box->setLayout(m_layout);

}

void HVGroup::notifyUpdate() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    //if (m_interface.m_conditions->getHVPMTReadState(0)) {
    //    m_on_btn->hide();
    //    m_off_btn->show();
    //} else {
    //    m_off_btn->hide();
    //    m_on_btn->show();
    //}
    for (int hv_id = 0; hv_id < m_interface.m_conditions->getNHVPMT(); hv_id++) {
        if (std::abs(m_interface.m_conditions->getHVPMTReadValue(hv_id) - m_interface.m_conditions->getHVPMTSetValue(hv_id))/float(m_interface.m_conditions->getHVPMTSetValue(hv_id)) > 0.05) {
            m_hventries.at(hv_id).readValue_label->setStyleSheet("QLabel { background-color : red; }");
        }
        else {
            m_hventries.at(hv_id).readValue_label->setStyleSheet("QLabel { background-color : green; }");
        }
        m_hventries.at(hv_id).readValue_label->setText(QString::number(m_interface.m_conditions->getHVPMTReadValue(hv_id)));
        m_hventries.at(hv_id).readCurrent_label->setText(QString::number(m_interface.m_conditions->getHVPMTReadCurrent(hv_id)));
    }

}

void HVGroup::switchON() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    // Propagate the new states to the Condition manager
    // Condition manager will tell setup manager to actually set the right state
    for (std::size_t id = 0; id < m_hventries.size(); id++) {
        bool state = m_hventries.at(id).cb_set_state->isChecked();
        m_interface.m_conditions->setHVPMTState(id, state);
        m_interface.m_conditions->propagateHVPMTState(id);
    }
    //m_on_btn->hide();
    //m_off_btn->show();
}

void HVGroup::switchOFF() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    // Update Condition manager with new HV states
    for (std::size_t id = 0; id < m_hventries.size(); id++) {
        m_interface.m_conditions->setHVPMTState(id, 0);
        m_interface.m_conditions->propagateHVPMTState(id);
    }
    //m_on_btn->show();
    //m_off_btn->hide();
}

void HVGroup::setHV() {
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getHVLock());

    // Update Condition manager with new HV set values
    for (std::size_t id = 0; id < m_hventries.size(); id++) {
        HVEntry hventry = m_hventries.at(id);
        m_interface.m_conditions->setHVPMTValue(id, hventry.sb_set_value->value());
        int new_value = m_interface.m_conditions->propagateHVPMTValue(id);
        hventry.setValue_label->setText(QString::number(new_value));
    }
}

void HVGroup::setRunning() {
    //for (HVEntry hventry: m_hventries) {
    //    hventry.sb_set_value->hide();
    //    hventry.setValue_label->show();
    //}
    //m_set->setDisabled(1);
}

void HVGroup::setNotRunning() {
    for (HVEntry hventry: m_hventries) {
        hventry.sb_set_value->show();
        hventry.setValue_label->hide();
    }
    m_set->setDisabled(0);
}
