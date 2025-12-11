#ifndef SUPERVISORWINDOW_H
#define SUPERVISORWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class SupervisorWindow;
}
QT_END_NAMESPACE

class SupervisorWindow : public QMainWindow
{
    Q_OBJECT

public:
    SupervisorWindow(QWidget *parent = nullptr);
    ~SupervisorWindow();

private:
    Ui::SupervisorWindow *ui;
};
#endif // SUPERVISORWINDOW_H
