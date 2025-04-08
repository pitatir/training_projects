#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Обработчик сигнала для SIGINT
void handlerSIGINT(int signum) {
    printf("Обработчик SIGINT\n");
    system("ps -s");
    exit(0);
}

// Обработчик сигнала для SIGQUIT
void handlerSIGQUIT(int signum) {
    printf("Обработчик SIGQUIT\n");
}

int main() {
    // Cтруктура для старого обработчика сигнала
    struct sigaction old_act;
    // Cтруктура для нового обработчика сигнала
    struct sigaction act;
    // Сброс маски сигналов
    sigemptyset(&act.sa_mask);
    // Не используем специальных флагов при обработке сигнала
    act.sa_flags = 0;

    sigset_t mask1;
    sigset_t mask2;
    sigemptyset(&mask1);
    sigemptyset(&mask2);

    // Установка обработчика сигнала для SIGINT
    // Задание пользовательской функции обработки сигнала
    act.sa_handler = handlerSIGINT;
    // Связываем обработчик сигнала SIGINT с обработчиком act и old_act
    // В случае ошибки
    if (sigaction(SIGINT, &act, &old_act) == -1) {
        printf("Ошибка sigaction\n");
        exit(1);
    }

    // Установка обработчика сигнала для SIGQUIT
    // Задание пользовательской функции обработки сигнала
    act.sa_handler = handlerSIGQUIT;
    // Связываем обработчик сигнала SIGQUIT с обработчиком act
    // В случае ошибки
    if (sigaction(SIGQUIT, &act, NULL) == -1) {
        printf("Ошибка sigaction\n");
        exit(1);
    }
    // Маскирование SIGQUIT (4)
    // добавляем SIGQUIT в маску
    sigaddset(&mask1, SIGQUIT);
    // сигналы в маске будут заблокированы
    if (sigprocmask(SIG_BLOCK, &mask1, NULL) == -1) {
        printf("Ошибка sigprocmask\n");
        exit(1);
    }

    // Маскирование SIGUSR1 (200)
    // добавляем SIGUSR1 в маску
    sigaddset(&mask2, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask2, NULL) == -1) {
        printf("Ошибка sigprocmask\n");
        exit(1);
    }

    // Вывод информации об обработчиках сигналов
    printf("SIGINT:  обработчик %p; старый обработчик %p, маска NULL\n", handlerSIGINT, old_act.sa_handler);
    printf("SIGQUIT: обработчик %p; старый обработчик NULL; маска ", handlerSIGQUIT);
    for (int i = 1; i <= 32; i++) {
        if (sigismember(&mask1, i)) {
            printf("%d ", i);

        }
    }
    printf("\n");
    printf("SIGUSR1: обработчик SIG_DFL; старый обработчик NULL; маска ");
    for (int i = 1; i <= 32; i++) {
        if (sigismember(&mask2, i)) {
            printf("%d ", i);

        }
    }
    printf("\n");

/*    // Вызываем сигналы с задержкой
    printf("\nВызов сигнала SIGQUIT\n");
    kill(getpid(), SIGQUIT);
    system("ps -s");
    sleep(1);*/

    printf("\nВызов сигнала SIGUSR1\n");
    kill(getpid(), SIGUSR1);
    system("ps -s");
    sleep(1);

    printf("\nВызов сигнала SIGINT\n");
    kill(getpid(), SIGINT);
    sleep(1);

    return 0;
}
