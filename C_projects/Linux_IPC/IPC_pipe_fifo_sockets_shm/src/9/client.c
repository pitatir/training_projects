#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main() {
    printf("Открываем канал 1 для чтения\n");
    // открываем первый канал для чтения
    int chan1 = open("channel1", O_RDONLY);
    // в случае ошибки
    if (chan1 == -1) {
        printf("Ошибка открытия канала 1 для чтения\n");
         // удаляем первый канал
        unlink("channel1");
        // удаляем первый канал 
        unlink("channel2");
        exit(1);
    }
    printf("Открываем канал 2 для записи\n");
    // открываем второй канал для чтения
    int chan2 = open("channel2", O_WRONLY);
    // в случае ошибки
    if (chan2 == -1) {
        printf("Ошибка открытия канала 2 для записи\n");
         // удаляем первый канал
        unlink("channel1");
        // удаляем первый канал 
        unlink("channel2");
        exit(1);
    }

    // имя файла для чтения
    char fileName[100];
    // очищаем буфер
    bzero(fileName, 100);

    printf("Читаем имя файла из канала 1\n");
    // читаем имя файла из первого канала для чтения
    int res = (int) read(chan1, fileName, 100);
    // в случае ошибки
    if (res <= 0) {
        printf("Ошибка чтения имени файла из канала 1\n");
         // удаляем первый канал
        unlink("channel1");
        // удаляем первый канал 
        unlink("channel2");
        exit(1);
    }

    printf("Открываем файл %s\n", fileName);
    // открываем файл на чтение
    FILE *f = fopen(fileName, "r");
    // в случае ошибки
    if (!f) {
        printf("Ошибка открытия файла %s\n", fileName);
         // удаляем первый канал
        unlink("channel1");
        // удаляем первый канал 
        unlink("channel2");
        exit(1);
    }

    printf("Читаем данные из файла и пишем в канал 2\n");
    // читаем из файла и пишем во второй канал
    char buf[100];
    // пока файл не пуст
    while (!feof(f)) {
        // читаем данные из файла
        res = (int) fread(buf, sizeof(char), 100, f);
        // пишем их в канал 2
        write(chan2, buf, res);
    }
    // закрываем файл
    fclose(f);
    printf("Закрываем каналы\n");
    // закрываем первый канал 
    close(chan1);
    // закрываем второй канал
    close(chan2);

    printf("Работа клиента завершена\n");
    return 0;
}
