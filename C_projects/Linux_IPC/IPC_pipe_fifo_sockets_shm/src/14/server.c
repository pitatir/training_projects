#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"

// Массив числа принятых пакетов каждым сокетом
volatile long long counts[1000];
// Количество сокетов
volatile int socketCount = 0;

// Функция получения сообщения
int readFix(int sock, char *buf, int bufSize, int flags) {
    // Длина сообщения
    size_t msgLength = 0;
    // Получение длины сообщения
    int res = (int) recv(sock, &msgLength, sizeof(size_t), flags | MSG_WAITALL);
    // Если не удалось получить, то вернуть код
    if (res <= 0)
        return res;
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
    int res = (int) send(sock, &msgLength, sizeof(unsigned), flags);
    // Если не удалось отправить, то вернуть результат
    if (res <= 0)
        return res;
    // Отправить само сообщение и вернуть результат
    return (int) send(sock, buf, msgLength, flags);
}

// Обработчик клиента
void *clientHandler(void *args) {
    // Увеличение числа сокетов
    socketCount++;
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
        // Получение пакета
        res = readFix(sock, buf, 100, 0);
        // В случае ошибки - выход из потока
        if (res <= 0) {
            perror("Ошибка получения данных клиента");
            pthread_exit(NULL);
        }
        // Увеличение количества принятых пакетов
        counts[sock - 4]++;
        // При получении 100 пакетов - вывод сообщения
        if (counts[sock - 4] == 100) {
            printf("Клиент %d передал 100 пактов данных\n", sock - 3);
        }
    }
}

// Обработчик прерывания
void sigintHandler() {
    // Вывод количества принятых пакетов каждым сокетом
    printf("\nКаждый из %d клиентов передал 100 пакетов данных, кроме:\n", socketCount);
    // Проверяем все переданные сокеты
    for (int n = 0; n < socketCount; n++) {
        if (counts[n] != 100) {
            printf("Клиента %d - передано %lld пакетов данных\n", n + 1, counts[n]);
        }
    }
    // Сброс буфера
    fflush(stdout);
    // Установка диспозиции по умолчанию
    signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv) {
    int port;
    if (argc < 2) {
        printf("Используем порт по умолчанию: %d\n\n", DEF_PORT);
        port = DEF_PORT;
    } else {
        port = atoi(argv[2]);
    }

    // Установка обработчика сигнала
    signal(SIGINT, sigintHandler);

    // Инициализация структуры адреса прослушивателя входящих соединений
    struct sockaddr_in listenerInfo;
    listenerInfo.sin_family = AF_INET;
    listenerInfo.sin_port = htons(port);
    listenerInfo.sin_addr.s_addr = htonl(INADDR_ANY);

    // Инициализация сокета
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    // Выход в случае ошибки
    if (listener < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }
    // Привязка сокета к адресу
    int res = bind(listener, (struct sockaddr *) &listenerInfo, sizeof(listenerInfo));
    // Выход в случае ошибки
    if (res < 0) {
        perror("Ошибка привязки сокета");
        close(listener);
        exit(1);
    }
    // Включение режима прослушивания
    res = listen(listener, 10);
    // Выход в случае ошибки
    if (res) {
        perror("Ошибка прослушивания");
        close(listener);
        exit(1);
    }

    // Основной цикл работы
    while (1) {
        // Принятие входящего соединения и создание сокета
        int client = accept(listener, NULL, NULL);
        // Инициализация потока для обработки сокета
        pthread_t thread;
        res = pthread_create(&thread, NULL, clientHandler, (void *) (client));
        // Вывод сообщения об ошибке
        if (res) {
            perror("Ошибка создания потока");
            close(listener);
            pthread_exit(NULL);
        }
    }
    return 0;
}

