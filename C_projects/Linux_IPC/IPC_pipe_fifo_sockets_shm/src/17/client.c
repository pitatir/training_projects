#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shm.h"

char buf[100];

int main() {
    const char keyFile[] = "keyFile";

    // на основе файла и proj_id, который может задаваться буквой,
    // создается ключ для обеспечения разделяемой памяти
    key_t key;
    if ((key = ftok(keyFile, 'Q')) < 0) {
        printf("Ошибка создания ключа из файла keyFile и id 'Q'\n");
        exit(1);
    }

    // создаем shm с разрешениями 0666
    int shmemory;
    if ((shmemory = shmget(key, sizeof(Message), 0666)) < 0) {
        printf("Ошибка создания общей памяти\n");
        exit(1);
    }

    // присоединяем shm в наше адресное пространство, NULL означает,
    // что будет автоматически определен и использован подходящий адрес
    Message *p_msg;
    if ((p_msg = (Message *) shmat(shmemory, NULL, 0)) < 0) {
        printf("Ошибка присоединения памяти\n");
        exit(1);
    }

    // создется набор семафоров, соответствующий ключу key
    int semaphore;
    if ((semaphore = semget(key, 2, 0666)) < 0) {
        printf("Ошибка создания семафоров\n");
        exit(1);
    }
    printf("Создан разделенный бинарный семафор\n");

    for (;;) {
        printf("\nВведите сообщение или пустую строку для завершения работы\n");

        // чтение сообщения
        bzero(buf, 100);
        fgets(buf, 100, stdin);

        // проверка на пустую строку
        if (strlen(buf) == 1 && buf[0] == '\n') {
            break;
        };
        // ждем пока семафор для записи не станет 1, т.е. можно писать
        if (semop(semaphore, writeEna, 1) < 0) {
            printf("Ошибка изменения семафора\n");
            exit(1);
        }
        // семафор для записи стал 0
        printf("Захват семафора \"запись разрешена\"\n");
        // запись сообщения в разделяемую память
        sprintf(p_msg->buf, "%s", buf);
        printf("Сообщение клиента записано в разделяемую память\n");

        // устнавливаем семафор для чтения в 1, т.е. можно читать
        if (semop(semaphore, setReadEna, 1) < 0) {
            printf("Ошибка изменения семафора\n");
            exit(1);
        }
        printf("Освобождение семафора \"чтение разрешено\"\n");
    }

    return 0;
}

