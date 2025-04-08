#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

#define DEF_KEY_FILE "key.txt"

// Структура сообщения
typedef struct {
    // тип сообщения
    long type;
    // текст сообщения
    char buf[100];
} Message;

// идентификатор очереди сообщений
int queue;
// дескрипто канала 1
int chan1;

// Обработчик сигнала SIGINT
void intHandler(int sig) {
    // вызываем обработчик по умолчанию
    signal(sig, SIG_DFL);
    printf("\nРабота клиента 2 завершена\n");
    unlink("channel1");
    close(chan1);
    exit(0);
}

int main(int argc, char **argv) {
    // имя файла с ключом
    char keyFile[100];
    // сообщение
    Message mes;
    // возвращаемое функцией значение
    int res;

    // ПОЛУЧЕНИЕ ДОСТУПА К ОЧЕРЕДИ С ПОМОЩЬЮ КЛЮЧА
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

    // ПРИВЯЗЫВАНИЕ ПОЛЬЗОВАТЕЛЬСКОГО ОБРАБОТЧИКА СИГНАЛА SIGINT
    signal(SIGINT, intHandler);

    // СОЗДАНИЕ КАНАЛА 1 ДЛЯ ИНДИКАЦИИ РАБОТЫ КЛИЕНТА 1
    printf("Создание канала 1 для индикации работы клиента 1\n");
    // создаем первый именнованный канал
    // устанавливаем для всех права на чтение и на запись
    res = mknod("channel1", S_IFIFO | 0666, 0);
    // в случае ошибки
    if (res) {
        printf("Ошибка создания канала 1\n");
        // удаляем первый канал
        unlink("channel1");
        exit(1);
    }

    // МОНИТОРИНГ КЛИЕНТА 1 ЧЕРЕЗ КАНАЛ 1
    // получаем дескриптор файла
    chan1 = open("channel1", O_RDONLY);
    // в случае ошибки
    if (chan1 == -1) {
        printf("Ошибка открытия канала channel1\n");
        unlink("channel1");
        exit(1);
    }

    // создаем структуру события для отслеживания
    struct epoll_event event;
    // задаем дескриптор для отслеживания
    event.data.fd = chan1;
    // задаем событие для отслеживания
    event.events = EPOLLIN | EPOLLHUP;

    // создание файлового дескриптора epoll
    int epoll_fd = epoll_create1(0);
    // в случае ошибки
    if (epoll_fd == -1) {
        printf("Ошибка создания файлового дескриптора epoll\n");
        unlink("channel1");
        close(chan1);
        exit(1);
    }

    // связываем структуру event с файлом
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, chan1, &event);
    // в случае ошибки
    if (ret == -1) {
        printf("Ошибка epoll_ctl\n");
        unlink("channel1");
        close(chan1);
        close(epoll_fd);
        exit(1);
    }

    // флаги, отвечающие за единчтвенный вывод сообщения о событии
    int epollin = 0;
    int epollhup = 0;
    // бесконечный цикл
    while (1) {
        // хранение отслеживаемых событий
        struct epoll_event events[2];
        // количество реально произошедших отслеживаемых событий
        int num_events = epoll_wait(epoll_fd, events, 2, -1);
        // в случае ошибки
        if (num_events == -1) {
            printf("Ошибка epoll_wait\n");
            unlink("channel1");
            close(chan1);
            close(epoll_fd);
            exit(1);
        }
        // проверяем все произошедшие события
        for (int i = 0; i < num_events; i++) {
            // если произошло событие EPOLLHUP
            if (events[i].data.fd == chan1 && events[i].events & EPOLLHUP && !epollhup) {
                epollhup = 1;
                printf("EPOLLHUP: канал 1 закрылся, клиент 1 закончил свою работу\n");
                // задаем сообщению тип 3
                mes.type = 3L;
                // очищение буфера
                bzero(mes.buf, 100);
                // запись сообщения
                sprintf(mes.buf, "Клиент 1 закончил свою работу");
                // дописываем символ конца строки
                mes.buf[strlen(mes.buf)] = '\0';
                // отправляем это сообщение
                res = msgsnd(queue, (void *) &mes, sizeof(Message), 0);
                // в случае ошибки
                if (res != 0) {
                    printf("Ошибка отправки сообщения\n");
                    unlink("channel1");
                    close(chan1);
                    close(epoll_fd);
                    exit(1);
                }
                // если произошло событие EPOLLIN
            } else if (events[i].data.fd == chan1 && events[i].events & EPOLLIN && !epollin) {
                epollin = 1;
                printf("EPOLLIN: канал 1 открылся, подключился клиент 1\n");
                // задаем сообщению тип 2
                mes.type = 2L;
                // очищение буфера
                bzero(mes.buf, 100);
                // запись сообщения
                sprintf(mes.buf, "Клиент 1 подключился");
                // дописываем символ конца строки
                mes.buf[strlen(mes.buf)] = '\0';
                // отправляем это сообщение
                res = msgsnd(queue, (void *) &mes, sizeof(Message), 0);
                // в случае ошибки
                if (res != 0) {
                    printf("Ошибка отправки сообщения\n");
                    unlink("channel1");
                    close(chan1);
                    close(epoll_fd);
                    exit(1);
                }
            }
        }
    }

    close(epoll_fd);
    return 0;
}


