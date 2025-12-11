#include "supervisorwindow.h"
#include "ui_supervisorwindow.h"

SupervisorWindow::SupervisorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SupervisorWindow)
{
    ui->setupUi(this);
}

SupervisorWindow::~SupervisorWindow()
{
    delete ui;
}
