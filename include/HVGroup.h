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

class HVGroup: public QGroupBox {
    friend class Interface;
    
    Q_OBJECT

    public:
        /*
         * Constructor
         * LOCKS: HV
         */
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

    private slots:
        //void setRunning();
        //void setNotRunning();
        
        /*
         * Switch on all HVs (propagate to ConditionManager)
         * LOCKS: HV
         */
        void switchON();
        
        /* 
         * Switch off all HVs (propagate to ConditionManager)
         * LOCKS: HV
         */
        //void switchOFF();
        
        /*
         * Set all HV voltages (propagate to ConditionManager)
         * LOCKS: HV
         */
        void setHV();

    private:
        void notifyUpdate();
      
        //QGroupBox *m_box;
        std::vector<HVEntry> m_hventries;
        QPushButton *m_set;
        QPushButton *m_on_btn;
        QPushButton *m_off_btn;
        QGridLayout *m_layout;

        Interface& m_interface;
};
