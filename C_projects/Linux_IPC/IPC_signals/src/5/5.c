#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// Флаги выполнения
int rtFlag, blockFlag, nestedFlag = 0;

// Обработчик сигнала
void sigusrHandler(int sig, siginfo_t *info, void *ptr) {
    // Вывод информационного сообщения
    printf("Дочерний процесс поймал сигнал %d с данными %d.\n",
           sig, info->si_value.sival_int);
    // Если включена вложенность
    if (nestedFlag) {
        // Формирование данных
        union sigval val;
        val.sival_int = info->si_value.sival_int * 2;
        // Если включены сигналы РВ
        if (rtFlag) {
            // Если сигнал был отправлен родителем -> отправить другой сигнал
            if (sig == SIGRTMIN) {
                sigqueue(getpid(), SIGRTMIN + 1, val);
            }
            // Иначе
        } else {
            // Если сигнал был отправлен родителем -> отправить другой сигнал
            if (sig == SIGUSR1) {
                sigqueue(getpid(), SIGUSR2, val);
            }
        }
    }
    // Приостановка на 3 секунды
    sleep(3);
    // Вывод информационного сообщения
    printf("Дочерний процесс закончил обработку сигнала %d с данными %d!\n",
           sig, info->si_value.sival_int);
}

// Код потомка
void child() {
    // Вывод информационного сообщения
    printf("Дочерний процесс: pid = %d, ppid = %d\n", getpid(), getppid());
    // Формирование структуры sigaction
    struct sigaction act, oldact;
    // Обработчик сигнала
    act.sa_sigaction = sigusrHandler;
    // Установка маски и флагов
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    // Если используются сигналы РВ
    if (rtFlag) {
        // Если используется блокировка -> обновление маски
        if (blockFlag) {
            sigaddset(&act.sa_mask, SIGRTMIN);
            sigaddset(&act.sa_mask, SIGRTMIN + 1);
        }
        // Установка обработчиков
        sigaction(SIGRTMIN, &act, &oldact);
        sigaction(SIGRTMIN + 1, &act, &oldact);
    }
        // Иначе
    else {
        // Если используется блокировка -> обновление маски
        if (blockFlag) {
            sigaddset(&act.sa_mask, SIGUSR1);
            sigaddset(&act.sa_mask, SIGUSR2);
        }
        // Установка обработчиков
        sigaction(SIGUSR1, &act, &oldact);
        sigaction(SIGUSR2, &act, &oldact);
    }

    // Вывод информационного сообщения
    printf("Обработчики сигналов изменены дочерним процессом\n");
    // Ожидание сигналов
    while (1) {
        pause();
    }
}

// Код родителя
void parent(pid_t child) {
    // Приостановка на 1 секунду
    sleep(1);
    // Вывод информационного сообщения
    printf("Отправка сигналов дочернему процессу\n\n");
    // Отправка 6 сигналов
    for (int i = 0; i < 6; i++) {
        // Приостановка на 1 секунду
        sleep(1);
        // Формирование данных сигнала
        union sigval val;
        val.sival_int = i;
        // Если не используется ветвление
        if (!nestedFlag) {
            // Отправка сигналов РВ / обычных сигналов
            if (rtFlag)
                sigqueue(child, SIGRTMIN + i % 2, val);
            else if (i % 2)
                sigqueue(child, SIGUSR2, val);
            else
                sigqueue(child, SIGUSR1, val);
        } else {
            // Отправка сигнала РВ / обычного сигнала
            if (rtFlag)
                sigqueue(child, SIGRTMIN, val);
            else
                sigqueue(child, SIGUSR1, val);
        }
    }
    // Приостановка на 30 секунд
    sleep(30);
    // Вывод информационного сообщения
    printf("Заканчиваем дочерний процесс\n\n");
    // Остановка потомка
    kill(child, SIGTERM);
    exit(0);
}


int main() {
    // Буфер для символа
    char input;

    // Ввод флага сигналов РВ
    printf("Использовать сигналы реального времени? [y/n] ");
    input = getchar();
    if (input == 'y')
        rtFlag = 1;
    getchar();

    // Ввод флага блокировки
    printf("Блокировать ли сигналы при обработке другого сигнала? [y/n] ");
    input = getchar();
    if (input == 'y')
        blockFlag = 1;
    getchar();

    // Ввод флага ветвления
    printf("Использовать вложенную обработку сигналов? [y/n] ");
    input = getchar();
    if (input == 'y')
        nestedFlag = 1;

    printf("\n");

    // Вывод информационного сообщения
    printf("Родительский процесс: pid = %d\n", getpid());

    // Вызов fork() и переход к функциям
    pid_t pid = fork();
    if (pid == 0) {
        child();
    } else {
        parent(pid);
    }
}

