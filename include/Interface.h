#pragma once

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QTimer>

#include <thread>
#include <memory>

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

        bool isRunning() const { return running; }

    public slots:
        void updateConditionLog();
        void notifyUpdate();

    private slots:
        void startLoggingManager();
        void stopLoggingManager();
        void showDiscriSettingsWindow();

    private:
      
        void setCounter(int i);

        HVGroup* m_hv_group;
        
        // Use a timer to refresh the interface periodically
        QTimer* m_timer;
        
        std::shared_ptr<LoggingManager> m_logging_manager;
        std::shared_ptr<ConditionManager> m_conditions;
        std::thread thread_handler;

        bool running;

        QPushButton *m_discriTunerBtn;
};
