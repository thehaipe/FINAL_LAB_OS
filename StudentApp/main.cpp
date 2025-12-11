#include "studentwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StudentWindow w;
    w.show();
    return a.exec();
}
