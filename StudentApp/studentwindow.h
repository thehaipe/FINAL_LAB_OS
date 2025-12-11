#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class StudentWindow;
}
QT_END_NAMESPACE

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    StudentWindow(QWidget *parent = nullptr);
    ~StudentWindow();

private:
    Ui::StudentWindow *ui;
};
#endif // STUDENTWINDOW_H
