#ifndef SHM_H
#define SHM_H

#include <sys/sem.h>

typedef struct {
    long type;
    char buf[100];
} Message;

// семафоры для чтения
static struct sembuf setReadEna[1] = {0, 1, 0};
static struct sembuf readEna[1] = {0, -1, 0};

// семафоры для записи
static struct sembuf setWriteEna[1] = {1, 1, 0};
static struct sembuf writeEna[1] = {1, -1, 0};

#endif //SHM_H

