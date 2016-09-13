#include <QWidget>
#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QCloseEvent>
#include <QLabel>

#include <vector>

class Interface;

class DiscriSettingsWindow : public QDialog {
    friend class Interface;

    Q_OBJECT
    
    public:
        /*
         * Constructor
         * LOCKS: Discri
         */
        DiscriSettingsWindow(Interface& m_interface);
        
        void closeEvent(QCloseEvent *event);
        
        struct DiscriChannel {
            QLabel *label;
            QCheckBox *included;
            QSpinBox *threshold;
            QSpinBox *width;
        };

    private slots:
        /*
         * Propagate new settings to ConditionManager
         * LOCKS: Discri
         */
        void propagate();

    private:
        std::vector<DiscriChannel> m_discriChannels;
        Interface& m_interface;
        QGroupBox *m_box;
        QSpinBox *m_box_majority;
        QPushButton *m_btn_propagate;

};

