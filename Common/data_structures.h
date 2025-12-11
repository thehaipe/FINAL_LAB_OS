#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <cstring>

// --- КОНСТАНТИ ---
// Назви для семафорів та пам'яті
#define SHARED_MEMORY_KEY "sharedMemory"
#define SEMAPHORE_KEY     "semaphoreForOurLab" //Якщо інша програма створить семафор з таким ім'ям, буде зле, тому так

// Ліміти
const int MAX_IDEAS = 20;       // Скільки ідей влазить на дошку
const int MAX_TEXT_LEN = 128;   // Довжина тексту однієї ідеї

// --- СТРУКТУРИ ---

// Одна ідея
struct Idea {
    int id;                     // Порядковий номер
    int studentPid;             // Хто придумав (ID процесу)
    char text[MAX_TEXT_LEN];    // Текст
    int votes;                  // Кількість голосів

    Idea() : id(0), studentPid(0), votes(0) {
        std::memset(text, 0, MAX_TEXT_LEN);
    }
};

// Стан Дошки
struct SharedBoard {
    bool isVotingStarted;       // true, коли керівник зупинив час і почав голосування
    bool isFinished;            // true, коли все закінчилось
    int ideaCount;              // Поточна кількість ідей
    Idea ideas[MAX_IDEAS];      // Масив всіх ідей


    int bestIdeaIds[3];         // Результати

    SharedBoard() : isVotingStarted(false), isFinished(false), ideaCount(0) {
        for(int i=0; i<3; ++i) bestIdeaIds[i] = -1;
    }
};

#endif // DATA_STRUCTURES_H
