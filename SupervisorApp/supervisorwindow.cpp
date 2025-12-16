#include "supervisorwindow.h"
#include "ui_supervisorwindow.h"

SupervisorWindow::SupervisorWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SupervisorWindow)
{
    ui->setupUi(this);
}

SupervisorWindow::~SupervisorWindow()
{
    delete ui;
}

void SupervisorWindow::onUpdateTimerTick()
{
    // Тут буде код оновлення таймера
}

void SupervisorWindow::onGameTimerTick()
{
    // Тут буде логіка ігрового часу
}

void SupervisorWindow::on_btnFinish_clicked()
{
    // Тут буде код для кнопки "Завершити"
}
