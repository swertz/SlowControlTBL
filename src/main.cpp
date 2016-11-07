#include <QApplication>

#include "Interface.h"
#include "Utils.h"

int main(int argc, char **argv) {
    QApplication my_app(argc, argv);
 
    Arguments m_args(argc, argv);
    Interface interface(m_args);

    interface.show();

    int status = my_app.exec();

    return status;
}
