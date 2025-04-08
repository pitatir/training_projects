#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEF_PORT 8888

// Функция получения сообщения
int readFix(int sock, char *buf, int bufSize, int flags) {
    // Длина сообщения
    size_t msgLength = 0;
    // Получение длины сообщения
    int res = (int) recv(sock, &msgLength, sizeof(size_t), flags | MSG_WAITALL);
    // Если не удалось получить, то вернуть код
    if (res <= 0) {
        return res;
    }
    // Если длина сообщения больше размера буфера
    if (res > bufSize) {
        // Вывод сообщения об ошибке и завершение программы
        printf("Ошибка: длина сообщения больше размера буфера\n");
        exit(1);
    }
    // Получение сообщения и возврат полученного значения
    return (int) recv(sock, buf, msgLength, flags | MSG_WAITALL);
}

// Функция отправки данных
int sendFix(int sock, char *buf, int flags) {
    // Длина сообщения
    size_t msgLength = strlen(buf) + 1;
    // Отправка длины сообщения
    int res = (int) send(sock, &msgLength, sizeof(size_t), flags);
    // Если не удалось отправить, то вернуть результат
    if (res <= 0) {
        return res;
    }
    // Отправить само сообщение и вернуть результат
    return (int) send(sock, buf, msgLength, flags);
}

// Обработчик клиента
void *clientHandler(void *args) {
    // Приведения аргумента к номеру сокета
    int sock = (int) args;
    // Буфер для сообщения
    char buf[100];
    // Результат
    int res;

    // Основной цикл работы
    while (1) {
        // Очистка буфера
        bzero(buf, 100);
        // Получение сообщения
        res = readFix(sock, buf, 100, 0);
        // В случае ошибки
        if (res <= 0) {
            perror("Ошибка получения данных клиента");
            pthread_exit(NULL);
            exit(0);
        }
        // Вывод сообщения
        printf("Клиент отправил: %s\n", buf);
        // Отправка сообщения обратно клиенту
        res = sendFix(sock, buf, 0);
        // В случае ошибки
        if (res <= 0) {
            perror("Ошибка отправки данных");
            pthread_exit(NULL);
        }
    }
}

int main(int argc, char **argv) {
    int port;
    if (argc < 2) {
        printf("Используем порт по умолчанию: %d\n", DEF_PORT);
        port = DEF_PORT;
    } else {
        port = atoi(argv[2]);
    }

    // Инициализация структуры заданным адресом прослушивателя входящих соединений
    struct sockaddr_in listenerInfo;
    listenerInfo.sin_family = AF_INET;
    listenerInfo.sin_port = htons(port);
    listenerInfo.sin_addr.s_addr = htonl(INADDR_ANY);

    // Вывод адреса в строковом виде
    struct in_addr display = {INADDR_ANY};
    printf("\nАдрес сокета %s:%d (%s:%d)\n",
           inet_ntoa(listenerInfo.sin_addr), listenerInfo.sin_port,
           inet_ntoa(display), port);

    // Инициализация сокета
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    // В случае ошибки
    if (listener < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }
    // Привязка сокета к адресу
    int res = bind(listener, (struct sockaddr *) &listenerInfo, sizeof(listenerInfo));
    // В случае ошибки
    if (res < 0) {
        perror("Ошибка привязки сокета");
        close(listener);
        exit(1);
    }
    // Включение режима прослушивания
    res = listen(listener, 5);
    // В случае ошибки
    if (res) {
        perror("Ошибка прослушивания");
        close(listener);
        exit(1);
    }

    // основной цикл работы
    while (1) {
        // Принятие входящего соединения и создание сокета
        int client = accept(listener, NULL, NULL);
        // Инициализация потока для обработки сокета
        pthread_t thread;
        res = pthread_create(&thread, NULL, clientHandler, (void *) (client));
        // В случае ошибки
        if (res) {
            printf("Ошибка создания потока\n");
            close(listener);
            pthread_exit(NULL);
        }
    }
    return 0;
}

