#include <QString>

#include "HVGroup.h"
#include "Interface.h"

HVGroup::HVGroup(Interface& m_interface):
    m_interface(m_interface),
    QWidget(&m_interface) {

        m_box = new QGroupBox("HV Settings", this);

        m_layout = new QGridLayout();

        int temp_iterator = 0;
        for (int setHVValue : m_interface.getConditions().getHVPMTSetValues()) {
            HVEntry hventry;
            const QString label = "HV " + QString::number(temp_iterator) + " value:";
            hventry.label = new QLabel(label);
            hventry.spin_box = new QSpinBox();
            hventry.spin_box->setRange(0, 2000);
            hventry.spin_box->setWrapping(1);
            hventry.spin_box->setSingleStep(1);
            hventry.spin_box->setValue(setHVValue);
            hventry.value_label = new QLabel(std::to_string(hventry.spin_box->value()).c_str());
            hventry.value_label->hide();
            //connect(hventry.spin_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &HVGroup::valueChanged);
            m_hventries.push_back(hventry);
            m_layout->addWidget(hventry.label, temp_iterator, 0);
            m_layout->addWidget(hventry.spin_box, temp_iterator, 1);
            m_layout->addWidget(hventry.value_label, temp_iterator, 1);
            temp_iterator++;
        }

        m_on_btn = new QPushButton("Switch On");
        //connect(m_on_btn, &QPushButton::clicked, [&](){ m_interface.getSetupManager()->switchHVPMTON(); });
        connect(m_on_btn, &QPushButton::clicked, this, &HVGroup::switchON);
        m_layout->addWidget(m_on_btn, temp_iterator, 0);

        m_off_btn = new QPushButton("Switch OFF");
        connect(m_off_btn, &QPushButton::clicked, this, &HVGroup::switchOFF);
        m_layout->addWidget(m_off_btn, temp_iterator, 0);
        m_off_btn->hide();

        m_set = new QPushButton("Set");
        connect(m_set, &QPushButton::clicked, this, &HVGroup::setHV);
        m_layout->addWidget(m_set, temp_iterator, 1);
        m_box->setLayout(m_layout);

}

//void HVGroup::valueChanged(int m_hv) {
//    if(m_interface.isRunning()) {
//        //m_spin_hv0->setValue(m_interface.getConditions().getHVPMTSetValues()[0]);
//        return;
//    }
//}

void HVGroup::switchON() {
    m_interface.getSetupManager()->switchHVPMTON();
    m_on_btn->hide();
    m_off_btn->show();
}

void HVGroup::switchOFF() {
    m_interface.getSetupManager()->switchHVPMTOFF();
    m_off_btn->hide();
    m_on_btn->show();
}

void HVGroup::setHV() {
    // Update Condition manager with new HV set values
    for (size_t id = 0; id < m_hventries.size(); id++) {
        HVEntry hventry = m_hventries.at(id);
        hventry.value_label = new QLabel(QString::number(hventry.spin_box->value()));
        m_interface.getConditions().setHVPMTValue(id, hventry.spin_box->value());
    }
    // Set physically the HV values
    m_interface.getSetupManager()->setHVPMT();
}

void HVGroup::setRunning() {
    for (HVEntry hventry : m_hventries) {
        hventry.spin_box->hide();
        hventry.value_label->show();
    }
    m_set->setDisabled(1);
}

void HVGroup::setNotRunning() {
    for (HVEntry hventry : m_hventries) {
        hventry.spin_box->show();
        hventry.value_label->hide();
    }
    m_set->setDisabled(0);
}
