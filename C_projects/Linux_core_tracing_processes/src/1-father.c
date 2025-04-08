#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // Вывод сообщения
    printf("Родительский процесс: pid = %d ppid = %d\n", getpid(), getppid());

    // Если процесс дочерний
    if (fork() == 0)
        // Запустить дочерний процесс с аргументом
        execl("/home/pitatir/LR7/1s", "/home/pitatir/LR7/1s", NULL);

    // Ожидание завершения дочернего процесса
    wait(NULL);
    // Вывод сообщения
    printf("Процесс pid = %d завершен\n", getpid());
    return 0;
}

