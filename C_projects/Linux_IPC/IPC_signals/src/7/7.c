#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <stdlib.h>

void handler(int signal, siginfo_t *info, void *context) {
    printf("Дочерний процесс поймал сигнал SIGRTMIN №%d\n", info->si_value.sival_int);
    usleep(1000);
}

int main() {
    setbuf(stdout, NULL);
    pid_t child = fork();
    if (child == 0) {
        struct sigaction act;
        // Установка обработчика сигналов
        act.sa_sigaction = handler;
        // Сброс флагов
        sigemptyset(&act.sa_mask);
        // Флаг дополнительной информации о сигнале
        act.sa_flags = SA_SIGINFO;
        //  Назначение обработчика
        if (sigaction(SIGRTMIN, &act, NULL) == -1) {
            printf("Ошибка sigaction\n");
            exit(1);
        }

        while (1);
    } else {
        sleep(1);
        //  Отправляение дочернему процессу 15 сигналов
        for (int i = 0; i < 8; i++) {
            union sigval data;
            data.sival_int = i;
            printf("Родительский процесс отправил сигнал SIGRTMIN №%d\n", i);
            sigqueue(child, SIGRTMIN, data);
            sleep(1);
        }
        printf("\n");
        sleep(1);
        // Убийство дочернего процесса
        kill(child, SIGKILL);
        wait(NULL);
    }
}

