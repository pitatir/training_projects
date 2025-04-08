#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define DEF_CLIENTS_AMT 1000

// Массив числа принятых пакетов каждым сокетом
volatile long long *counts;
// Количество сокетов
volatile int totalSocketCount = 0;

// Обработчик прерывания
void sigHandler() {
    printf("\n");
    // Вывод общего количества пакетов, принятых каждым сокетом
    for (int i = 0; i < DEF_CLIENTS_AMT; ++i) {
        printf("Клиент %d передал %lld пактов данных\n", i + 1, counts[i]);
    }
    // Вывод общего количества принятых пакетов
    printf("\nОбщее количество принятых пакетов данных: %d из %d (%d%%)\n", totalSocketCount, DEF_CLIENTS_AMT * 100,
           totalSocketCount / DEF_CLIENTS_AMT);
    free((long long *) counts);
    // Восстановление обработчика
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

    // Установка обработчика прерывания
    signal(SIGINT, sigHandler);
    counts = (volatile long long *) calloc(1000, sizeof(long long));

    // Инициализация структуры адреса
    struct sockaddr_in listenerInfo;
    listenerInfo.sin_family = AF_INET;
    listenerInfo.sin_port = htons(port);
    listenerInfo.sin_addr.s_addr = htonl(INADDR_ANY);

    // Создание сокета
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    // Выход в случае ошибки
    if (sock < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }
    // Привязка сокета к адресу
    int res = bind(sock, (struct sockaddr *) &listenerInfo, sizeof(listenerInfo));
    // Выход в случае ошибки
    if (res < 0) {
        perror("Ошибка привязки сокета");
        close(sock);
        exit(1);
    }

    // Буфер сообщения
    char buf[100];
    int prev_client = 1001;
    // Основной цикл работы
    while (1) {
        // Получение сообщения
        ssize_t n = recvfrom(sock, buf, sizeof buf, 0, NULL, NULL);
        // Вывод в случае ошибки
        if (n < 0) {
            perror("Ошибка получения сообщения");
        }
        // Выход в случае получения слишком большого числа данных
        if (n > sizeof buf) {
            printf("Ошибка: длина сообщения больше размера буфера\n");
            exit(1);
        }
        // Увеличение общего количества принятых пакетов
        totalSocketCount++;     
        // Увеличение общего количества пакетов, принятых данным клиентом
        int ind = atoi(buf);
        counts[ind]++;
        if (totalSocketCount % 100 == 0){
            printf("Обработан клиент %d\n", ind + 1);
        }
    }
    return 0;
}

