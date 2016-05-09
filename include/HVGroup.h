#pragma once
#pragma once

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QPushButton>

#include <memory>

class Interface;

class HVGroup : public QWidget {
    friend class Interface;
    
    Q_OBJECT

    public:
        HVGroup(Interface& m_interface);
        virtual ~HVGroup() {}

    private slots:
        void valueChanged(int m_hv);
        void setHV();
        void setRunning();
        void setNotRunning();

    private:
      
        QGroupBox *m_box;
        QLabel *m_label;
        QLabel *m_value_label;
        QSpinBox *m_spin;
        QPushButton *m_set;
        QGridLayout *m_layout;

        Interface& m_interface;
};
