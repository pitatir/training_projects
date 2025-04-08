#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <stdbool.h>
#include <stdlib.h>

// Обработчик сигналов
void handler(int signal, siginfo_t *info, void *context) {
    printf("Дочерний процесс поймал сигнал РВ: %d\n", signal - SIGRTMIN);
    usleep(1000);
}

int main() {
    // Установка буферизации вывода в стандартный вывод на NULL, что позволяет немедленно выводить информацию
    setbuf(stdout, NULL);
    // Создание дочеренго процесса
    pid_t child = fork();
    if (child == 0) {
        struct sigaction act;

        // Установка обработчика сигналов
        act.sa_sigaction = handler;
        // Сброс флагов
        sigemptyset(&act.sa_mask);  // Сброс флагов
        // Флаг дополнительной информации о сигнале
        act.sa_flags = SA_SIGINFO;

        for (int i = SIGRTMIN; i <= SIGRTMIN + 5; i++) {
            //  Назначение обработчика
            if (sigaction(i, &act, NULL) == -1) {
                printf("Ошибка sigaction\n");
                exit(1);
            }
        }
        // Бесконечный цикл ожидания
        while (true);
    } else {
        // Время на установку обработчика
        sleep(1);
        // Цикл отправки сигнала в реальном времени дочернему процессу
        for (int i = SIGRTMIN; i <= SIGRTMIN + 5; i += 2) {
            printf("Родительский процесс отправил сигнал РВ: %d\n", i - SIGRTMIN);
            kill(child, i);
        }
        for (int i = SIGRTMIN + 5; i >= SIGRTMIN; i -= 2) {
            printf("Родительский процесс отправил сигнал РВ: %d\n", i - SIGRTMIN);
            kill(child, i);
        }
        printf("\n");
        sleep(1);
        // Убийство дочернего процесса
        kill(child, SIGKILL);
        wait(NULL);
    }

}

