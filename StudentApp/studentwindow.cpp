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
    isConnected = false;

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
    if (semaphore->acquire()) {
        semaphore->release();
    } else {
        sharedMem->detach();
        return false;
    }

    isConnected = true;
    ui->lblStatus->setText(QString("PID: %1. Підключено. Етап 1: Генерація ідей.").arg(myPid));
    return true;
}

void StudentWindow::switchToVotingMode()
{
    ui->lblStatus->setText(QString("PID: %1. Етап 2: Голосування.").arg(myPid));
    ui->btnSubmit->setEnabled(false);
    ui->IdeaInLineEdit->setEnabled(false);

    // Якщо студент вже голосував, кнопка залишається вимкненою
    if (!hasVoted) {
        ui->btnVote->setEnabled(true);
    }
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

    SharedBoard * board = (SharedBoard*)sharedMem->data();

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


void StudentWindow::loadCandidatesList(SharedBoard *board)
{
    ui->CandidatesListWidget->clear();
    for(int i = 0; i < board->ideaCount; ++i) {
        ui->CandidatesListWidget->addItem(QString("#%1: %2")
                                              .arg(board->ideas[i].id)
                                              .arg(board->ideas[i].text));
    }
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
void StudentWindow::on_btnVote_clicked()
{
    if (!sharedMem->isAttached()) {
        QMessageBox::warning(this, "Помилка", "Не підключено до спільної пам'яті.");
        return;
    }

    // 1. Отримати список ВИБРАНИХ елементів
    QList<QListWidgetItem*> selectedItems = ui->CandidatesListWidget->selectedItems();
    int voteCount = selectedItems.size();

    // Перевірка, чи щось вибрано
    if (voteCount == 0) {
        QMessageBox::information(this, "Помилка", "Будь ласка, виберіть принаймні одну ідею для голосування.");
        return;
    }

    // 2. Блокуємо доступ до спільної пам'яті
    semaphore->acquire();
    SharedBoard *board = (SharedBoard*)sharedMem->data();

    // 3. Перевірка: чи голосування вже почалося?
    if (!board->isVotingStarted) {
        QMessageBox::information(this, "Помилка", "Генерація ідей ще триває. Голосування неможливе.");
        semaphore->release();
        return;
    }

    int successfulVotes = 0;

    // 4. Перебираємо УСІ вибрані елементи
    for (QListWidgetItem* selectedItem : selectedItems)
    {
        // 4a. Витягуємо ID ідеї з поля Qt::UserRole елемента списку
        int ideaIdToVote = selectedItem->data(Qt::UserRole).toInt();

        // 4b. Перебираємо ідеї у спільній пам'яті та шукаємо відповідний ID
        for (int i = 0; i < board->ideaCount; i++) {
            if (board->ideas[i].id == ideaIdToVote) {

                // *** ОСНОВНА ЗМІНА: Збільшення голосу для кожного вибраного елемента ***
                board->ideas[i].votes++;
                successfulVotes++;
                break; // Знайшли, переходимо до наступного вибраного елемента
            }
        }
    }

    // 5. Знімаємо блокування
    semaphore->release();

    // 6. Повідомлення про результат
    if (successfulVotes > 0) {
        QMessageBox::information(this, "Успіх",
                                 QString("Ваш голос зараховано! Ви проголосували за %1 ідею(ї).").arg(successfulVotes));

        // Опціонально: Після голосування можна скинути вибір,
        // щоб студент не проголосував двічі, випадково натиснувши кнопку знову.
        for (QListWidgetItem* item : selectedItems) {
            item->setSelected(false);
        }

    } else {
        QMessageBox::critical(this, "Помилка", "Не вдалося знайти жодної вибраної ідеї для голосування.");
    }
}

void StudentWindow::on_CandidatesListWidget_itemSelectionChanged(){
    QList<QListWidgetItem*> selected = ui->CandidatesListWidget->selectedItems();
    const int MAX_VOTES = 3;
    if (selected.size() > MAX_VOTES) {
        QMessageBox::warning(this, "Обмеження вибору",
                             QString("Ви можете вибрати не більше %1 ідей.").arg(MAX_VOTES));
        for (int i = MAX_VOTES; i < selected.size(); i++) {
            selected.at(i)->setSelected(false);
        }
    }
}

