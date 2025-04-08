#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <wait.h>

int main() {
    // файл для записи
    char fileToRead[] = "readFile.txt";
    // файл для чтения
    char fileToWrite[] = "writeFile.txt";
    // дискрипторы для неименнованного канала
    int descryptors[2];
    // создаем неименнованный канал
    if (pipe(descryptors) < 0) {
        printf("Не удалось создать неименнованный канал\n");
        exit(1);
    }
    printf("Создан неименнованный канал\n");

    // дочерний процесс
    if (fork() == 0) {
        // закрываем неименнованный канал на чтение - будем только писать
        close(descryptors[0]);
        // открываем файл на чтение
        FILE *f = fopen(fileToRead, "r");
        if (f == NULL) {
            printf("Не удалось открыть файл для чтения\n");
            exit(1);
        }
        printf("Дочерний процесс читает данные из файла %s и записывает их в канал\n", fileToRead);
        // сообщение
        char buf[100];
        // пока не закончится файл
        while (!feof(f)) {
            // читаем данные из файла
            int n = (int) fread(buf, sizeof(char), 100, f);
            // записываем в неименнованный канал
            write(descryptors[1], buf, n);
        }
        // закрываем файл
        fclose(f);
        // закрываем неименнованный канал
        close(descryptors[1]);
        return 0;
    }

    // родительский процесс
    // закрываем неименнованный канал на запись - будем только читать
    close(descryptors[1]);
    // открываем файл
    FILE *f = fopen(fileToWrite, "w");
    if (f == NULL) {
        printf("Не удалось открыть файл для записи\n");
        exit(1);
    }
    wait(NULL);
    // сообщение
    char buf[100];
    printf("Родительский процесс читает данные из канала и записывает их в файл %s\n", fileToWrite);
    printf("\nСообщения, полученные родительским процессом из канала:\n");
    while (1) {
        // читаем данные из пайпа
        bzero(buf, 100);
        int n = (int) read(descryptors[0], buf, 100);

        // если пайп опустел заканчиваем работу
        if (!n) {
            break;
        }
        // выводим сообщение из пайпа
        printf("%s\n", buf);
        // записываем в файл
        fwrite(buf, sizeof(char), n, f);
    }

    // закрываем файл
    fclose(f);
    // закрываем пайп
    close(descryptors[0]);
    return 0;
}

