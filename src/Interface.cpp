#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

#include "Interface.h"

Interface::Interface(QWidget *parent): 
    QWidget(parent),
    label(nullptr),
    m_setup(new Setup()) {
        std::cout << "Creating Interface. Qt version: " << qVersion() << "." << std::endl;

        QPushButton *startBtn = new QPushButton("Start", this);
        QPushButton *stopBtn = new QPushButton("Stop", this);
        QPushButton *plsBtn = new QPushButton("+ 100", this);
        QPushButton *minBtn = new QPushButton("- 100", this);
        label = new QLabel("0", this);
        QPushButton *quit = new QPushButton("Quit", this);

        QGridLayout *grid = new QGridLayout(this);
        grid->addWidget(startBtn, 0, 0);
        grid->addWidget(stopBtn, 0, 1);
        grid->addWidget(plsBtn, 1, 0);
        grid->addWidget(minBtn, 1, 1);
        grid->addWidget(quit, 2, 0);
        grid->addWidget(label, 2, 1);

        setLayout(grid);

        connect(startBtn, &QPushButton::clicked, this, &Interface::startLoggingManager);
        connect(stopBtn, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(plsBtn, &QPushButton::clicked, this, &Interface::onPlus);
        connect(minBtn, &QPushButton::clicked, this, &Interface::onMinus);
        connect(quit, &QPushButton::clicked, this, &Interface::stopLoggingManager);
        connect(quit, &QPushButton::clicked, qApp, &QApplication::quit);

        resize(400, 300);
        //showFullScreen();
        //showMaximized();
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

void Interface::startLoggingManager(){
    if(m_logging_manager.get())
        return;

    m_logging_manager = std::make_shared<LoggingManager>(*this);
    thread_handler = std::thread(&LoggingManager::run, std::ref(*m_logging_manager));
}
    
void Interface::stopLoggingManager(){
    if(!m_logging_manager.get())
        return;

    m_logging_manager->stop();
    thread_handler.join();
    m_logging_manager.reset();

    m_setup->setCounter(0);
    notifyUpdate();
}
    
void Interface::onPlus(){
    //int val = label->text().toInt() + 100;
    m_setup->lock();
    int val = m_setup->getCounter() + 100;
    label->setText(QString::number(val));
    m_setup->setCounter(val);
    m_setup->unlock();
}

void Interface::onMinus(){
    //int val = label->text().toInt() - 100;
    m_setup->lock();
    int val = m_setup->getCounter() - 100;
    label->setText(QString::number(val));
    m_setup->setCounter(val);
    m_setup->unlock();
}
