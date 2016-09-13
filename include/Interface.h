#pragma once

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QTimer>
#include <QSpinBox>

#include <thread>
#include <memory>
#include <atomic>

class ConditionManager;
class LoggingManager;
class HVGroup;
class DiscriSettingsWindow;

class Interface : public QWidget {
    friend class HVGroup;
    friend class DiscriSettingsWindow;
    
    Q_OBJECT

    public:
        Interface(QWidget *parent = 0);

        virtual ~Interface() {}

        ConditionManager& getConditions();

        /*
         * Define states of the state machine.
         * Possible transitions (defined in source file):
         *  idle -> configured
         *  configured -> running
         *  running -> idle
         *  configured -> idle
         */
        enum class State {
            idle,
            configured,
            running
        };
        
        /*
         * Get current state
         */
        Interface::State getState() const { return m_state; }

        /*
         * Convert the state to a string
         */
        static std::string stateToString(Interface::State state);

        /*
         * Check if transition from `state_from` to `state_to` is allowed
         */
        static bool checkTransition(Interface::State state_from, Interface::State state_to);

    public slots:
        void updateConditionLog();
        void notifyUpdate();

    private slots:
        /*
         * Configure the run, change state to "configured"
         * LOCKS: TTC, TDC
         */
        void configureRun();
       
        /*
         * Start the run, change state to "running"
         */
        void startRun();
        
        /*
         * Stop the run, change state to "idle"
         */
        void stopRun();
        
        /*
         * Quit the application: depending on current state, stop run or not
         */
        void quit();
        
        void showDiscriSettingsWindow();

    private:
      
        void setCounter(int i);

        HVGroup* m_hv_group;
        
        // Use a timer to refresh the interface periodically
        QTimer* m_timer;
        
        std::shared_ptr<LoggingManager> m_logging_manager;
        std::shared_ptr<ConditionManager> m_conditions;
        std::thread thread_handler;
        
        /*
         * Change  state to `state`.
         * Throws exception if transition from current state is not allowed.
         * Returns:
         *  - `true` if state was changed successfully
         *  - `false` if interface  was already in the requested state
         */
        bool setState(Interface::State state);

        // Transitions are defined in .cc file
        static const std::vector< std::pair<State, State> > m_transitions;
        std::atomic<State> m_state;
        
        QPushButton *m_configureBtn;
        QPushButton *m_startBtn;
        QPushButton *m_stopBtn;
        
        QSpinBox *m_runNumberSpin;
        QLabel *m_runNumberLabel;
        QPushButton *m_discriTunerBtn;

        QSpinBox *m_triggerChannel_box;
        QSpinBox *m_triggerRandom_box;
        QLabel *m_tdc_backPressure_label;
        QLabel *m_tdc_fatal_label;
        QLabel *m_tdc_ok_label;
        QLabel *m_tdc_eventCounter_label;
};
