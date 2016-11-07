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

#include "Utils.h"

class ConditionManager;
class LoggingManager;
class HVGroup;
class Trigger_TDC_Group;
class DiscriSettingsWindow;

class Interface : public QWidget {
    friend class HVGroup;
    friend class Trigger_TDC_Group;
    friend class DiscriSettingsWindow;
 
    Q_OBJECT

    public:
        Interface(Arguments m_args, QWidget* parent = 0);

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

    private slots:
        void updateConditionLog();
        void notifyUpdate();

        /*
         * Configure the run, change state to "configured"
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

        Arguments m_args;
      
        void setCounter(int i);

        HVGroup* m_hv_group;
        Trigger_TDC_Group* m_ttc_tdc_group;
        
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
};
