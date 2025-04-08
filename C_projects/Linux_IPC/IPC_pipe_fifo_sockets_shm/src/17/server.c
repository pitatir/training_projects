#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shm.h"

int shmemory;
int semaphore;

void intHandler() {
    // удаляем shm и семафоры
    if (shmctl(shmemory, IPC_RMID, 0) < 0) {
        printf("Ошибка удаления общей памяти\n");
        exit(1);
    }
    if (semctl(semaphore, 0, IPC_RMID) < 0) {
        printf("Ошибка удаления семафора\n");
        exit(1);
    }

    printf("\nЗавершение работы программы\n");
    exit(0);
}

int main() {
    const char keyFile[] = "keyFile";
    printf("Создание сервера\n");
    // на основе файла и proj_id, который может задаваться буквой,
    // создается ключ для обоспечения разделяемой памяти
    key_t key;
    if ((key = ftok(keyFile, 'Q')) < 0) {
        printf("Ошибка создания ключа из файла keyFile и id 'Q'\n");
        exit(1);
    }

    // создаем shm с разрешениями 0666
    if ((shmemory = shmget(key, sizeof(Message), IPC_CREAT | 0666)) < 0) {
        printf("Ошибка создания общей памяти\n");
        exit(1);
    }

    // присоединяем shm в наше адресное пространство, NULL означает,
    // что будет автоматически определен и использован подходящий адрес
    Message *p_msg;
    if ((p_msg = (Message *) shmat(shmemory, NULL, 0)) < 0) {
        printf("Ошибка при присоединении памяти\n");
        exit(1);
    }

    // устанавливаем обработчик сигнала
    signal(SIGINT, intHandler);

    // создаем группу из 2 семафоров, соответствующую ключу key
    // первый показывает, что можно читать
    // второй показывает, что можно писать
    if ((semaphore = semget(key, 2, IPC_CREAT | 0666)) < 0) {
        printf("Ошибка создания семафоров\n");
        kill(getpid(), SIGINT);
    }
    printf("Создан разделенный бинарный семафор\n");
    // устнавливаем семафор для записи в 1, т.е. можно писать
    if (semop(semaphore, setWriteEna, 1) < 0) {
        kill(getpid(), SIGINT);
    }

    // основной цикл работы
    for (;;) {

        // ждем пока семафор для чтения не станет 1, т.е. можно читать
        if (semop(semaphore, readEna, 1) < 0) {
            kill(getpid(), SIGINT);
        }
        // семафор для чтения стал 0
        printf("\nЗахват семафора \"чтение разрешено\"\n");
        // читаем сообщение от клиента
        printf("Сообщение клиента: %s", p_msg->buf);

        // устанавливаем семафор для записи в 1, т.е. можно писать
        if (semop(semaphore, setWriteEna, 1) < 0) {
            kill(getpid(), SIGINT);
        }
        printf("Освобождение семафора \"запись разрешена\"\n");
    }
}

