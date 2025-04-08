#!/bin/bash

# путь к исполняемому файлу с кодом
prog=/home/pitatir/LR7/src/prog3
# директория с необходимыми для трассировки файлами
dir=/sys/kernel/debug/tracing

# включаем трассировку
sysctl kernel.ftrace_enabled=1

# передаем в фильтр желаемые для отображения системные функции 
echo '*getppid' > ${dir}/set_ftrace_filter
echo '*wait' >> ${dir}/set_ftrace_filter

# включаем опцию отображения PID процесса и задачи
echo funcgraph-proc > ${dir}/trace_options

# выбираем графический трассировщик функций
echo function_graph > ${dir}/current_tracer

# включаем анализ событий переключения планировщика
# т.е. переключений контекста
echo 1 > events/sched/sched_switch/enable 
# включаем анализ событий пробуждения планировщика 
# т.е. запуска процесса после перехода в режим ожидания или блокировки
echo 1 > events/sched/sched_wakeup/enable

# включаем обновление кольцевого буфера
echo 1 > ${dir}/tracing_on 
# запускаем программу для трассировки
sudo taskset --cpu-list 0 ${prog}
# выключаем обновление кольцевого буфера
echo 0 > ${dir}/tracing_on

# выводим содержимое файла с трассировкой
less ${dir}/trace
