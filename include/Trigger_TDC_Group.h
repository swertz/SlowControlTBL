#pragma once

#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>

class Interface;

class Trigger_TDC_Group: public QGroupBox {
    friend class Interface;
    
    Q_OBJECT

    public:
        /*
         * Constructor
         */
        Trigger_TDC_Group(Interface& m_interface);
        
        virtual ~Trigger_TDC_Group() {}
        
    private:
        /*
         * Update the status flag (OK, back-pressure, fatal)
         * LOCKS: TDC
         */
        void notifyUpdate();

        /*
         * Called when the run is configured
         * - propagate trigger settings to ConditionManager
         * - configure the TDC
         * - Show the event counter
         * LOCKS: TTC, TDC
         */
        void atConfigureRun();

        /*
         * Called when the run is started
         * - start TDC reading
         * - start TTC
         * - Show the TDC label
         * LOCKS: TTC, TDC
         */
        void atStartRun();

        /*
         * Called when the run is stopped
         * - stop TDC reading
         * - stop TTC
         * - Hide the TDC label & event counter
         * LOCKS: TTC, TDC
         */
        void atStopRun();
        
        Interface& m_interface;
        
        QSpinBox *m_triggerChannel_box;
        QSpinBox *m_triggerRandom_box;
        QLabel *m_tdc_backPressure_label;
        QLabel *m_tdc_fatal_label;
        QLabel *m_tdc_ok_label;
        QLabel *m_tdc_eventCounter_label;
};
