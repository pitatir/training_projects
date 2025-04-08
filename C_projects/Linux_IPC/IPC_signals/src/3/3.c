#include <stdio.h>
#include <signal.h>
#include <unistd.h>


// Обработчик сигнала SIGUSR1
void handlerUSR1(int sig) {
    if (sig == SIGUSR1) {
        // Выводим сообщение
        printf("Пойман сигнал SIGUSR1\n");
    }
    sleep(20);
}


int main() {
    // Вызываем ненадежную обработку сигнала
    signal(SIGUSR1, handlerUSR1);
    while (1) {
        // Ставим программу на паузу
        // В случае получения сигнала SIGINT (Ctrl+C) программа завершится
        pause();
    }
    return 0;
}
