#include "studentwindow.h"
#include "ui_studentwindow.h"

StudentWindow::StudentWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StudentWindow)
{
    ui->setupUi(this);
    ui->CandidatesListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // 1. Ініціалізація IPC об'єктів
    sharedMem = new QSharedMemory(SHARED_MEMORY_KEY);
    semaphore = new QSystemSemaphore(SEMAPHORE_KEY);

    // 2. Ініціалізація логіки
    myPid = QCoreApplication::applicationPid();
    ui->lblStatus->setText(QString("PID: %1. Очікування підключення...").arg(myPid));
    ui->btnVote->setEnabled(false);

    // 3. Встановлення таймера моніторингу
    monitorTimer = new QTimer(this);
    connect(monitorTimer, &QTimer::timeout, this, &StudentWindow::onMonitorTick);
    // Починаємо моніторинг: спершу з швидкої спроби підключення
    monitorTimer->start(100);
}

StudentWindow::~StudentWindow()
{
    if (sharedMem->isAttached()) {
        sharedMem->detach();
    }
    delete sharedMem;
    delete ui;
}
bool StudentWindow::connectToIPC()
{
    if (isConnected) return true;

    // Спроба підключення до спільної пам'яті
    if (!sharedMem->isAttached()) {
        if (!sharedMem->attach()) {
            qDebug() << "Не вдалося підключитися до Shared Memory:" << sharedMem->errorString();
            return false;
        }
    }

    isConnected = true;
    ui->lblStatus->setText(QString("PID: %1. Підключено. Етап 1: Генерація ідей.").arg(myPid));
    return true;
}

void StudentWindow::onMonitorTick()
{
    // Спершу перевіряємо підключення
    if (!connectToIPC()) {
        ui->lblStatus->setText(QString("PID: %1. Очікування підключення...").arg(myPid));
        return;
    }

    // 1. Взяти семафор для безпечного читання
    if (!semaphore->acquire()) return;

    SharedBoard *board = (SharedBoard*)sharedMem->data();

    if (board) {
        // Перевірка стану гри
        if (board->isFinished) {
            monitorTimer->stop();
            ui->lblStatus->setText("Сесія завершена. Переможці визначені.");
            ui->btnSubmit->setEnabled(false);
            ui->btnVote->setEnabled(false);
        }
        else if (board->isVotingStarted) {
            monitorTimer->stop(); // Зупиняємо моніторинг і переходимо в режим голосування
            switchToVotingMode();
            loadCandidatesList(board);
        }
    }

    // 2. Відпустити семафор
    semaphore->release();
}


void StudentWindow::on_btnSubmit_clicked()
{
    if (!hasSubmitted){
        QString IdeaInText = ui->IdeaInLineEdit->text();

        // Перевірка наявності даних
        if (IdeaInText.isEmpty()) {
            qDebug() << "Ідея не може бути пустою.";
            return;
        }

        // 1. Блокування спільної пам'яті
        if (!sharedMem->lock()) {
            qDebug() << "Помилка блокування shared memory.";
            return;
        }

        // 2. Отримання вказівника та перевірка
        SharedBoard *board = (SharedBoard*)sharedMem->data();
        if (!board) {
            sharedMem->unlock();
            qDebug() << "Вказівник на SharedBoard недійсний.";
            return;
        }

        // Перевірка на переповнення
        if (board->ideaCount >= MAX_IDEAS) {
            sharedMem->unlock();
            qDebug() << "Спільна пам'ять для ідей заповнена.";
            return;
        }

        // 3. Розрахунок місця для запису
        int newIdeaIndex = board->ideaCount;
        Idea *newIdeaLocation = &(board->ideas[newIdeaIndex]);

        // 4. Запис метаданих
        newIdeaLocation->id = newIdeaIndex;
        newIdeaLocation->studentPid = myPid;

        // 5. Запис тексту ідеї
        QByteArray byteArray = IdeaInText.toUtf8();
        const char *cString = byteArray.constData();
        // Використовуємо sizeof для запобігання переповненню буфера
        qstrncpy(newIdeaLocation->text, cString, sizeof(newIdeaLocation->text));

        // 6. Оновлення лічильника
        board->ideaCount++;

        // 7. Розблокування
        sharedMem->unlock();
        hasSubmitted = true;
        qDebug() << "Ідея успішно записана в спільну пам'ять з ID:" << newIdeaIndex;
    } else {
        QMessageBox::warning(this, tr("Помилка"), tr("Ідея вже була відправлена"));
    }
}

