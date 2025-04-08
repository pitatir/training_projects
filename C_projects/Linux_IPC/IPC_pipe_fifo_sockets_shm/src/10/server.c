#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define DEF_KEY_FILE "key.txt"

// Структура сообщения
typedef struct {
    // тип сообщения
    long type;
    // текст сообщения
    char buf[100];
} Message;

int queue;

// Обработчик сигнала SIGINT
void intHandler(int sig) {
    // вызываем обработчик по умолчанию
    signal(sig, SIG_DFL);
    // удаление очереди сообщений
    if (msgctl(queue, IPC_RMID, 0) < 0) {
        printf("Ошибка удаления очереди\n");
        exit(1);
    }
    printf("\nРабота сервера завершена\n");
    exit(0);
}

int main(int argc, char **argv) {
    // имя файла с ключом
    char keyFile[100];
    // очищаем буфер
    bzero(keyFile, 100);
    // если не передан файл с ключом
    if (argc < 2) {
        printf("Используем файл с ключом по умолчанию %s\n", DEF_KEY_FILE);
        strcpy(keyFile, DEF_KEY_FILE);
    } else
        // в противном случае используем переданный файл
        strcpy(keyFile, argv[1]);
    // уникальный ключ, необходимый для задания очереди
    key_t key;
    // получение ключа
    // преобразование имени файла и идентификатора
    key = ftok(keyFile, 'Q');
    // в случае ошибки
    if (key == -1) {
        printf("Не получен ключ для файла %s и ID 'Q'\n", keyFile);
        exit(1);
    }
    // создание очереди сообщений
    queue = msgget(key, IPC_CREAT | 0666);
    // в случае ошибки
    if (queue < 0) {
        printf("Ошибка создания очереди\n");
        exit(4);
    }

    // связывание сигнала с его обработчиком
    signal(SIGINT, intHandler);

    Message mes;
    int res;
    // бесконечный цикл чтения сообщений
    for (;;) {
        // очищение буфера
        bzero(mes.buf, 100);
        // чтение сообщения
        res = (int) msgrcv(queue, &mes, sizeof(Message), 1L, 0);
        // в случае ошибки
        if (res < 0) {
            printf("Ошибка получения сообщения\n");
            kill(getpid(), SIGINT);
        }
        // вывод сообщения
        printf("Сообщение клиента: %s\n", mes.buf);
        sleep(3);
    }
    return 0;
}
