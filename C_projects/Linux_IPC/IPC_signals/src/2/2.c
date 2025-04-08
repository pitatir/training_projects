#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Надежный обработчик сигналов
void (*mysig(int signum, void (*handler)(int)))(int) {
    // Создание структуры для обработчика сигнала
    struct sigaction act;
    // Задание переданной пользовательской функции обработки сигнала
    act.sa_handler = handler;
    // Очищаем сигналы act.sa_mask, чтобы никакие другие сигналы не были заблокированы во время обработки текущего
    sigemptyset(&act.sa_mask);
    // Добавляем сигнал SIGINT в act.sa_mask, чтобы заблокировать сигнал SIGINT на время обработки текущего сигнала
    sigaddset(&act.sa_mask, SIGINT);
    sigaddset(&act.sa_mask, SIGUSR1);
    // Не используем специальных флагов при обработке сигнала
    act.sa_flags = 0;
    // Вызываем надежный обработчик сигнала signum с обработчиком act для их связывания
    // В случе ошибки
    if (sigaction(signum, &act, 0) < 0) {
        // Вывести сообщение об ошибке
        return SIG_ERR;
    }
    // Возвращаем указатель на функцию обработки сигнала
    return act.sa_handler;
}

// Обработчик сигнала SIGUSR1
void handlerUSR1(int sig) {
    if (sig == SIGUSR1) {
        // Выводим сообщение
        printf("Пойман сигнал SIGUSR1\n");
    } else {
        // Выводим сообщение
        printf("Получен сигнал %d, отличный от SIGUSR1\n", sig);
        return;
    }
    // Блокируем сигнал SIGUSR1, вызвав "засыпание" на 40 секунд
    sleep(40);
}


int main() {
    // Вызываем надежную обработку сигнала
    mysig(SIGUSR1, handlerUSR1);
    while (1) {
        // Ставим программу на паузу
        // В случае получения сигнала SIGINT (Ctrl+C) программа завершится
        pause();
    }
    return 0;
}
