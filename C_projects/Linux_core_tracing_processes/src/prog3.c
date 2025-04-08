#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main(void) {
    // Параметры планирования
    struct sched_param shed_parameters;
    // Pid завершенного процесса
    pid_t status;
    // Статус изменения планировщика
    int res;

    // Получаем параметры планирования текущего родительского процесса
    sched_getparam(0, &shed_parameters);

    // Установка приоритета планирования
    shed_parameters.sched_priority = 0;

    // Меняем политику планирования
    res = sched_setscheduler(0, SCHED_OTHER, &shed_parameters);
    // Если попытка оказалась неудачной
    if (res == -1) {
        // Вывод сообщения об ошибке
        perror("SCHED_SETSCHEDULER_1");
    }

    // Выводим измененную политику планирования
    printf("Политика планирования родительского процесса: ");
    switch (sched_getscheduler(0)) {
        case SCHED_FIFO:
            printf("SCHED_FIFO\n\n");
            break;
        case SCHED_RR:
            printf("SCHED_RR\n\n");
            break;
        case SCHED_OTHER:
            printf("SCHED_OTHER\n\n");
            break;
        case -1:
            perror("SCHED_GETSCHEDULER\n");
            break;
        default:
            printf("Неизвестная политика планирования\n\n");
    }

    // Создаем 32 дочерних процесса
    for (int i = 0; i < 32; i++)
        // Если процесс дочерний
        if (fork() == 0) {
            // Выводим сообщение
            printf("%d) Дочерний процесс: pid = %d ppid = %d\n", i, getpid(), getppid());

            // Выполняем длинный цикл
            for (long j = 0; j < 300000000; j++) {
                // Выводим сообщение о достижении середины подсчетов
                if (j == 150000000) {
                    printf("Checkpoint %d: процесс с pid = %d\n", i, getpid());
                }
            }
            // Выводим сообщение о завершении работы дочернего процесса
            printf("Процесс %i с pid = %i закончен\n", i, getpid());
            // Завершаем работу
            exit(0);
        }

    // Ждем выполнения всех дочерних процессов
    while ((status = wait(NULL)) > 0) {
        printf("Процесс завершен: pid = %d\n", status);
    }
    // Выводим сообщение о завершении работы родительского процесса
    printf("Завершение родительской программы pid = %d\n", getpid());
    return 0;
}

