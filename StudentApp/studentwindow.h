#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QTimer>
#include <QMessageBox>
#include <QListWidgetItem> // Потрібно для роботи зі списком

// Підключаємо спільні структури
#include "../Common/data_structures.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class StudentWindow;
}
QT_END_NAMESPACE

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    StudentWindow(QWidget *parent = nullptr);
    ~StudentWindow();

private slots:
    void on_btnSubmit_clicked(); // Натискання "Відправити ідею"
    void on_btnVote_clicked();   // Натискання "Проголосувати"
    void onMonitorTick();        // Таймер: перевірка стану "Дошки"
    void on_CandidatesListWidget_itemSelectionChanged();

private:
    Ui::StudentWindow *ui;

    // --- IPC ---
    QSharedMemory *sharedMem;
    QSystemSemaphore *semaphore;

    // --- Логіка ---
    QTimer *monitorTimer; // Опитує сервер (Керівника)
    int myPid;            // ID нашого процесу (щоб підписати ідею)

    // Флаги стану для локального інтерфейсу
    bool isConnected;  // Чи успішно ми підключились до пам'яті
    bool hasSubmitted; // Чи вже відправили ідею (щоб не спамити)
    bool hasVoted;     // Чи вже проголосували

    // --- Допоміжні методи ---
    bool connectToIPC();                         // Спроба підключення (викликається таймером)
    void switchToVotingMode();                   // Перемикання інтерфейсу на Етап 2
    void loadCandidatesList(SharedBoard *board); // Завантаження списку для голосування
};

#endif // STUDENTWINDOW_H
