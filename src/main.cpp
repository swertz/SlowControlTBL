#include <QApplication>

#include "Interface.h"

int main(int argc, char **argv) {
    QApplication my_app(argc, argv);
 
    Interface interface;

    interface.show();

    int status = my_app.exec();

    return status;
}
