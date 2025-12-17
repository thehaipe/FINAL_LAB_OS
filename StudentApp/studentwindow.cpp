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

    // 2. Ініціалізація логіки
    myPid = QCoreApplication::applicationPid();
    ui->lblStatus->setText(QString("PID: %1. Очікування підключення...").arg(myPid));
    ui->btnVote->setEnabled(false);
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

