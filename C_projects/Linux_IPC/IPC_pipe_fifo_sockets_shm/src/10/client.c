#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>

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
    printf("\nРабота клиента завершена\n");
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
    // получение доступа к созданной сервером очереди сообщений
    queue = msgget(key, 0);
    // в случае ошибки
    if (queue < 0) {
        printf("Ошибка доступа к очереди\n");
        exit(4);
    }

    // связывание сигнала с его обработчиком
    signal(SIGINT, intHandler);

    Message mes;
    int res;
    // бесконечный цикл отправки сообщений
    for (;;) {
        // задаем сообщению тип 1
        mes.type = 1L;
        // очищение буфера
        bzero(mes.buf, 100);
        // чтение сообщения из консоли
        fgets(mes.buf, 100, stdin);
        // дописываем символ конца строки
        mes.buf[strlen(mes.buf) - 1] = '\0';
        // отправляем это сообщение
        res = msgsnd(queue, (void *) &mes, sizeof(Message), 0);
        // в случае ошибки
        if (res != 0) {
            printf("Ошибка отправки сообщения\n");
            exit(1);
        }
    }
    return 0;
}

