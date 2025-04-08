#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define DEF_THREADS_AMT 100

pthread_mutex_t mutex;
char *addr;
int port;


// Рабочий поток
void *threadWorker(void *arg) {
    // Захват мьютекса
    pthread_mutex_lock(&mutex);
    // Получение номера потока из аргументов
    int id = (int) arg;
    // Инициализация структуры интернет адреса
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(addr);

    // Инициализация сокета
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    // Выход в случае ошибки
    if (sock < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    // Подключение к сокету
    int res = connect(sock, (struct sockaddr *) &peer, sizeof(peer));
    // Выход в случае ошибки
    if (res) {
        perror("Ошибка подключения к серверу");
        close(sock);
        exit(1);
    }

    // Освобождение мьютекса
    pthread_mutex_unlock(&mutex);

    // Инициализация буфера
    char buf[100];
    sprintf(buf, "%d", id);
    // Основной цикл работы - 100 итераций
    for (int i = 0; i < 100; i++) {
        // Отправка сообщения
        res = send(sock, buf, strlen(buf) + 1, 0);
        // Выход в случае ошибки
        if (res <= 0) {
            fprintf(stderr, "Поток %d: Ошибка отправки сообщения серверу", id);
            perror("");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    int threadAmt;
    // Проверка числа аргументов
    if (argc < 4) {
        printf("Используем порт по умолчанию: %d\n", DEF_PORT);
        port = DEF_PORT;
    } else {
        port = atoi(argv[3]);
    }
    if (argc < 3) {
        printf("Используем адрес по умолчанию: %s\n", DEF_IP);
        addr = DEF_IP;
    } else {
        addr = argv[2];
    }
    if (argc < 2) {
        printf("Используем количество потоков по умолчанию: %d\n", DEF_THREADS_AMT);
        threadAmt = DEF_THREADS_AMT;
    } else {
        threadAmt = atoi(argv[1]);
    }

    // Инициализация потоков
    for (int i = 0; i < threadAmt; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, threadWorker, (void *) i);
    }

    printf("\nСоздано %d потоков-клиентов\n", threadAmt);
    // Приостановка главного потока на 40 секунд
    sleep(20);
    printf("Конец работы программы\n");
    return 0;
}

