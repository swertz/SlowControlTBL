#include "HVGroup.h"
#include "Interface.h"

HVGroup::HVGroup(Interface& m_interface):
    m_interface(m_interface),
    QWidget(&m_interface) {

        m_box = new QGroupBox("HV Settings", this);

        m_label = new QLabel("HV value:");
        
        m_value_label = new QLabel("0");
        m_value_label->hide();

        m_spin = new QSpinBox(0);
        m_spin->setRange(0, 2000);
        m_spin->setSingleStep(1);

        m_set = new QPushButton("Set");

        m_layout = new QGridLayout();
        m_layout->addWidget(m_label, 0, 0);
        m_layout->addWidget(m_spin, 0, 1);
        m_layout->addWidget(m_value_label, 0, 1);
        m_layout->addWidget(m_set, 0, 2);
        m_box->setLayout(m_layout);

        connect(m_spin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &HVGroup::valueChanged);
        connect(m_set, &QPushButton::clicked, this, &HVGroup::setHV);
}

void HVGroup::valueChanged(int m_hv) {
    if(m_interface.isRunning()) {
        m_spin->setValue(m_interface.getSetup().getHV());
        return;
    }
}

void HVGroup::setHV() {
    if(m_interface.isRunning())
        return;

    m_interface.setHV(m_spin->value());
    m_value_label->setText(QString::number(m_spin->value()));
}

void HVGroup::setRunning() {
    m_spin->hide();
    m_set->hide();
    m_value_label->show();
}

void HVGroup::setNotRunning() {
    m_value_label->hide();
    m_set->show();
    m_spin->show();
}
