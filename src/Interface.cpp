#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>

#include <mutex>

#include "Interface.h"

Interface::Interface(QWidget *parent): 
    QWidget(parent),
    label(nullptr),
    m_setup(new Setup()),
    running(false) {
        std::cout << "Creating Interface. Qt version: " << qVersion() << "." << std::endl;

        QGridLayout *master_grid = new QGridLayout();

        QGroupBox *run_box = new QGroupBox("Run control");
        QGridLayout *run_grid = new QGridLayout();
        
        QPushButton *startBtn = new QPushButton("Start");
        QPushButton *stopBtn = new QPushButton("Stop");
        QPushButton *plsBtn = new QPushButton("+ 100");
        QPushButton *minBtn = new QPushButton("- 100");
        label = new QLabel("0");
        QPushButton *quit = new QPushButton("Quit");
        
        m_hv_group = new HVGroup(*this);

        run_grid->addWidget(startBtn, 0, 0);
        run_grid->addWidget(stopBtn, 0, 1);
        run_grid->addWidget(plsBtn, 1, 0);
        run_grid->addWidget(minBtn, 1, 1);
        run_grid->addWidget(quit, 2, 0);
        run_grid->addWidget(label, 2, 1);
        run_box->setLayout(run_grid);

        master_grid->addWidget(run_box, 0, 0);
        master_grid->addWidget(m_hv_group, 0, 1);

        setLayout(master_grid);

        connect(startBtn, &QPushButton::clicked, this, &Interface::startLoggingManager);
        connect(stopBtn, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(plsBtn, &QPushButton::clicked, this, &Interface::onPlus);
        connect(minBtn, &QPushButton::clicked, this, &Interface::onMinus);
        connect(quit, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(quit, &QPushButton::clicked, qApp, &QApplication::quit);

        resize(500, 200);
        //showFullScreen();
}

Setup& Interface::getSetup(){
    return *m_setup;
}

void Interface::notifyUpdate() {
    setCounter(m_setup->getCounter());
}

void Interface::setCounter(int i) {
    label->setText(QString::number(i));
}

void Interface::setHV(int hv_value) {
    m_setup->setHV(hv_value);
}

void Interface::startLoggingManager(){
    if(m_logging_manager.get())
        return;

    m_logging_manager = std::make_shared<LoggingManager>(*this);
    thread_handler = std::thread(&LoggingManager::run, std::ref(*m_logging_manager));

    m_hv_group->setRunning();

    running = true;
}
    
void Interface::stopLoggingManager(){
    if(!m_logging_manager.get())
        return;

    m_logging_manager->stop();
    thread_handler.join();
    m_logging_manager.reset();

    m_setup->setCounter(0);
    notifyUpdate();

    m_hv_group->setNotRunning();

    running = false;
}
    
void Interface::onPlus(){
    std::lock_guard<std::mutex> lock(m_setup->getLock());
    int val = m_setup->getCounter() + 100;
    label->setText(QString::number(val));
    m_setup->setCounter(val);
}

void Interface::onMinus(){
    std::lock_guard<std::mutex> lock(m_setup->getLock());
    int val = m_setup->getCounter() - 100;
    label->setText(QString::number(val));
    m_setup->setCounter(val);
}
