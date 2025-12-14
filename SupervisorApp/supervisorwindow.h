#ifndef SUPERVISORWINDOW_H
#define SUPERVISORWINDOW_H

#include <QMainWindow>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QTimer>
#include <QMessageBox>
#include <QListWidgetItem>

#include "../Common/data_structures.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SupervisorWindow; }
QT_END_NAMESPACE

class SupervisorWindow : public QMainWindow
{
    Q_OBJECT

public:
    SupervisorWindow(QWidget *parent = nullptr);
    ~SupervisorWindow();

private slots:
    void onUpdateTimerTick();  // Оновлення GUI
    void onGameTimerTick();    // Відлік 3-х хвилин
    void on_btnFinish_clicked(); // Кнопка завершення

private:
    Ui::SupervisorWindow *ui;

    // --- IPC ---
    QSharedMemory *sharedMem;
    QSystemSemaphore *semaphore;

    // --- Логіка часу ---
    QTimer *updateTimer;      // Опитування пам'яті
    QTimer *gameTimer;        // Секундомір
    int timeRemaining;        // Секунди до кінця (180 с)

    // --- Допоміжні ---
    void initIPC();           // Створення ресурсів
    void cleanupIPC();        // Очистка
    void showResults();       // Підрахунок голосів
};

#endif
