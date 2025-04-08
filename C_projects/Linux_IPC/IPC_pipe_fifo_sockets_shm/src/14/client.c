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
    int res = (int) send(sock, &msgLength, sizeof(size_t), flags);
    // Если не удалось отправить, то вернуть результат
    if (res <= 0)
        return res;
    // Отправить само сообщение и вернуть результат
    return (int) send(sock, buf, msgLength, flags);
}

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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // Выход в случае ошибки
    if (sock < 0) {
        perror("Ошибка создания сокета\n");
        exit(1);
    }

    // Подключение к сокету
    int res = connect(sock, (struct sockaddr *) &peer, sizeof(peer));
    // В случае ошибки
    if (res) {
        perror("Ошибка подключения к серверу");
        close(sock);
        exit(1);
    }

    // Освобождение мьютекса
    pthread_mutex_unlock(&mutex);

    // Инициализация буфера
    char buf[100];
    sprintf(buf, "Поток %d", id);
    // Основной цикл работы - 100 итераций
    for (int i = 0; i < 100; i++) {
        // Отправка сообщения
        res = sendFix(sock, buf, 0);
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
