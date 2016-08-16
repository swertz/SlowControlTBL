#include <QSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QCloseEvent>
#include <QLabel>
#include <QString>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "DiscriSettingsWindow.h"
#include "Interface.h"
#include "ConditionManager.h" 

DiscriSettingsWindow::DiscriSettingsWindow(Interface& m_interface):
    m_interface(m_interface),
    QDialog(&m_interface) {
        QVBoxLayout *discri_layout = new QVBoxLayout();
        m_box = new QGroupBox("Discri Settings", this);
        QGridLayout *discri_boxLayout = new QGridLayout();

        // First display the meaning of the settings on top of it
        int vPos_settingLabels = 0;

        int hPos_threshold = 1;
        QLabel *label_threshold = new QLabel("Threshold");
        label_threshold->setAlignment(Qt::AlignCenter);
        discri_boxLayout->addWidget(label_threshold, vPos_settingLabels, hPos_threshold);

        int hPos_width = 2;
        QLabel *label_width = new QLabel("Width");
        label_width->setAlignment(Qt::AlignCenter);
        discri_boxLayout->addWidget(label_width, vPos_settingLabels, hPos_width);

        int hPos_include = 3;
        QLabel *label_include = new QLabel("Include");
        label_include->setAlignment(Qt::AlignCenter);
        discri_boxLayout->addWidget(label_include, vPos_settingLabels, hPos_include);

        std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getDiscriLock());
        
        // Display the channel settings themselves
        int vPos_channel = 0;
        for (int dc_id = 0; dc_id < m_interface.m_conditions->getNDiscriChannels(); dc_id++) {
            DiscriChannel discriChannel;

            vPos_channel = dc_id+1;

            discriChannel.label = new QLabel("Discri channel " + QString::number(dc_id) + " : ");
            discri_boxLayout->addWidget(discriChannel.label, vPos_channel, 0);

            discriChannel.included = new QCheckBox();
            discriChannel.included->setStyleSheet("margin-left:50%; margin-right:50%;");
            discriChannel.included->setChecked(m_interface.m_conditions->getDiscriChannelState(dc_id));
            discri_boxLayout->addWidget(discriChannel.included, vPos_channel, hPos_include);

            discriChannel.threshold = new QSpinBox();
            discriChannel.threshold->setValue(m_interface.m_conditions->getDiscriChannelThreshold(dc_id));
            discri_boxLayout->addWidget(discriChannel.threshold, vPos_channel, hPos_threshold);

            discriChannel.width = new QSpinBox();
            discriChannel.width->setMaximum(200);
            discriChannel.width->setValue(m_interface.m_conditions->getDiscriChannelWidth(dc_id));
            discri_boxLayout->addWidget(discriChannel.width, vPos_channel, hPos_width);
            m_discriChannels.push_back(discriChannel);

        }

        // Display the number of channel required for a coincidence 

        int majorityPosition = vPos_channel + 1;
        QLabel *globalSettingsLabel = new QLabel("Global settings : ");
        discri_boxLayout->addWidget(globalSettingsLabel, majorityPosition+1, 0);

        QLabel *label_majority = new QLabel("Majority");
        discri_boxLayout->addWidget(label_majority, majorityPosition, 1);
        m_box_majority = new QSpinBox();
        m_box_majority->setRange(0, m_interface.m_conditions->getNDiscriChannels());
        m_box_majority->setWrapping(1);
        m_box_majority->setValue(m_interface.m_conditions->getChannelsMajority());
        m_box_majority->setAlignment(Qt::AlignCenter);
        discri_boxLayout->addWidget(m_box_majority, majorityPosition+1, 1);

        m_box->setLayout(discri_boxLayout);
        discri_layout->addWidget(m_box);

        // Button to propagate the settings
        m_btn_propagate = new QPushButton("Propagate");
        connect(m_btn_propagate, &QPushButton::clicked, this, &DiscriSettingsWindow::propagate);
        connect(m_btn_propagate, &QPushButton::clicked, &m_interface, &Interface::updateConditionLog);
        connect(m_btn_propagate, &QPushButton::clicked, [&](){ QCloseEvent *event = new QCloseEvent(); this->closeEvent(event); });
        discri_layout->addWidget(m_btn_propagate);

        setLayout(discri_layout);
    }

// Ensure that the proper actions are taken when clicking the exit button
// i.e. re-enabling the main window button opening this dialog box
void DiscriSettingsWindow::closeEvent(QCloseEvent *event) {
    m_interface.m_discriTunerBtn->setDisabled(0);
    m_interface.m_discriTunerBtn->setText("Discri Settings");
    QDialog::reject();
}

// Propagate the setting to the condition manager and to the setup
void DiscriSettingsWindow::propagate() {
        
    std::lock_guard<std::mutex> m_lock(m_interface.m_conditions->getDiscriLock());
    
    // Majority
    m_interface.m_conditions->setChannelsMajority(m_box_majority->value());
    
    for (int dc_id = 0; dc_id < m_interface.m_conditions->getNDiscriChannels(); dc_id++) {
        // Include channel or not
        m_interface.m_conditions->setDiscriChannelState(dc_id, m_discriChannels.at(dc_id).included->isChecked());
        // Width
        m_interface.m_conditions->setDiscriChannelWidth(dc_id, m_discriChannels.at(dc_id).width->value());
        // Threshold
        m_interface.m_conditions->setDiscriChannelThreshold(dc_id, m_discriChannels.at(dc_id).threshold->value());
    }
    
    // ask condition manager to propagate the settings to the setup
    m_interface.m_conditions->propagateDiscriSettings();
}
