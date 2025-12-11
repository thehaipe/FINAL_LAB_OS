#include "supervisorwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SupervisorWindow w;
    w.show();
    return a.exec();
}
