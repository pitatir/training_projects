#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"

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

int main(int argc, char **argv) {
    char *addr;
    int port;
    if (argc < 3) {
        printf("Используем порт по умолчанию: %d\n", DEF_PORT);
        port = DEF_PORT;
    } else {
        port = atoi(argv[2]);
    }
    if (argc < 2) {
        printf("Используем адрес по умолчанию: %s\n", DEF_IP);
        addr = DEF_IP;
    } else {
        addr = argv[1];
    }

    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Ошибка создания сокета\n");
        exit(1);
    }

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(addr);


    // Вывод адреса в строковом виде
    struct in_addr display = {INADDR_LOOPBACK};
    printf("\nАдрес сокета %s:%d (%s:%d)\n",
           inet_ntoa(peer.sin_addr), peer.sin_port,
           addr, port);

    // Присоединяемся к серверу
    int res = connect(sock, (struct sockaddr *) &peer, sizeof(peer));
    // В случае ошибки
    if (res) {
        perror("Ошибка подключения к серверу");
        close(sock);
        exit(1);
    }

    // Буфер под сообщение
    char buf[100];
    // Основной цикл работы
    while (1) {
        printf("\nВведите сообщение\n");
        // Очищение буфера
        bzero(buf, 100);
        // Считывание ввода
        fgets(buf, 100, stdin);
        // Добавление символа конца строки
        buf[strlen(buf) - 1] = '\0';
        // Если пользователь ничего не ввел
        if (strlen(buf) == 0) {
            // Завершаем работу
            printf("Конец работы клиента\n");
            return 0;
        }
        // Отправляем сообщение серверу
        res = sendFix(sock, buf, 0);
        // В случае ошибки
        if (res <= 0) {
            perror("Ошибка отправки сообщения серверу");
            close(sock);
            exit(1);
        }
        // Очищение буфера
        bzero(buf, 100);
        // Прием ответа сервера
        res = readFix(sock, buf, 100, 0);
        // В случае ошибки
        if (res <= 0) {
            perror("Ошибка получения сообщения от сервера");
            close(sock);
            exit(1);
        }
        printf("Ответ сервера: %s\n", buf);
    }
    return 0;
}
