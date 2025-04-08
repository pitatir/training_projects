#include <stdio.h>
#include <unistd.h>

int main() {
    // Вывод сообщения
    printf("Дочерний процесс: pid = %d ppid = %d\n", getpid(), getppid());
    // Приостановка процесса на 2 секунды
    sleep(2);
    return 0;
}
