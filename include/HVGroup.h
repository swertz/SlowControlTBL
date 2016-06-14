#pragma once
#pragma once

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QPushButton>
#include <QCheckBox>

#include <memory>
#include <cstddef>

class Interface;

class HVGroup : public QWidget {
    friend class Interface;
    
    Q_OBJECT

    public:
        HVGroup(Interface& m_interface);
        virtual ~HVGroup() {}
        
        struct HVEntry {
            QLabel *label;
            QLabel *setValue_label;
            QLabel *readValue_label;
            QLabel *readCurrent_label;
            QSpinBox *sb_set_value;
            QCheckBox *cb_set_state;
        };

    public:
        void notifyUpdate();

    private slots:
        void setRunning();
        void setNotRunning();
        void switchON();
        void switchOFF();
        void setHV();

    private:
      
        QGroupBox *m_box;
        std::vector<HVEntry> m_hventries;
        QPushButton *m_set;
        QPushButton *m_on_btn;
        QPushButton *m_off_btn;
        QGridLayout *m_layout;

        Interface& m_interface;
};
