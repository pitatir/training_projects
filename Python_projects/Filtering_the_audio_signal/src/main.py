import re

import matplotlib.pyplot as plt
import numpy as np

# Данные варианта
ORIG_SIGNAL_FILE = "signaldigit9.txt"
DURATION = 6.75  # Продолжительность записи

# Задаем сами
f = 1400  # Анализируемая частота, смотрим по графику
delta_f = 40  # Задаем произвольно сами. Лучше значение поменьше, чтобы добротность фильтра Q = f / delta_f была больше.
C = 1e-6  # Произвольное значение электроемкости конденсатора C = 1 мкФ
L = 1 / (C * (2 * np.pi * f) ** 2)  # Индуктивность катушки L = 0.013 Гн
R = delta_f / f * np.sqrt(L / C)  # Сопротивление резистора R = 3.3 Ом
FILTER_SCHEMA_FILE = "Схема фильтра.png"


# График исходного сигнала во временной области
def original_signal_plot(axs=None):
    if axs is None:
        fig, axs = plt.subplots(figsize=(25, 12.5))
    axs.set_title('Исходный сигнал', fontsize=38)
    axs.set_xlabel('Время, с', fontsize=34)
    axs.set_ylabel('Значение', fontsize=34)
    axs.grid(axis='both')
    axs.tick_params(axis='x', labelsize=30)
    axs.tick_params(axis='y', labelsize=30)
    axs.plot(timeline, original_signal, c='navy')
    if axs is None:
        plt.show()


# График спектра исходного сигнала
def original_spectrum_plot(axs=None):
    if axs is None:
        fig, axs = plt.subplots(figsize=(25, 12.5))
    axs.plot(frequencies[1:size // 2], np.abs(original_spectre[1:size // 2]), c='navy')
    axs.set_title('Спектр исходного сигнала', fontsize=38)
    axs.set_xlabel('Частота, Гц', fontsize=34)
    axs.set_ylabel('Амплитуда', fontsize=34)
    tics = np.arange(0, 22000, 2000)
    axs.set_xticks(tics)
    axs.tick_params(axis='x', labelsize=24)
    axs.tick_params(axis='y', labelsize=30)
    axs.grid(axis='both')
    axs.scatter(x=1390, y=350, c='maroon', s=100, zorder=2)
    axs.text(x=1200, y=25000, s='1400 Гц', fontsize=24, color='maroon', ha='center', va='top')
    axs.legend()
    if axs is None:
        plt.show()


# График фильтрованного сигнала во временной области
def filtered_signal_plot(axs=None):
    if axs is None:
        fig, axs = plt.subplots(figsize=(25, 12.5))
    axs.plot(timeline, filtered_signal, c='indigo')
    axs.set_title('Фильтрованный сигнал', fontsize=38)
    axs.set_xlabel('Время, с', fontsize=34)
    axs.set_ylabel('Значение', fontsize=34)
    axs.grid(axis='both')
    axs.tick_params(axis='x', labelsize=30)
    axs.tick_params(axis='y', labelsize=30)
    if axs is None:
        plt.show()


# График спектра фильтрованного сигнала
def filtered_spectrum_plot(axs=None):
    if axs is None:
        fig, axs = plt.subplots(figsize=(25, 12.5))
    axs.plot(frequencies[1:size // 2], np.abs(filtered_spectre[1:size // 2]), c='indigo', linewidth=4)
    axs.set_title('Спектр фильтрованного сигнала', fontsize=38)
    axs.set_xlabel('Частота, Гц', fontsize=34)
    axs.set_ylabel('Амплитуда', fontsize=34)
    tics = np.arange(0, 22500, 2500)
    tics = np.append(tics, f)
    axs.set_xticks(tics)
    axs.tick_params(axis='x', labelsize=24)
    axs.tick_params(axis='y', labelsize=30)
    axs.grid(axis='both')
    if axs is None:
        plt.show()


# Схема используемого фильтра
def draw_filter_schema(axs=None):
    if axs is None:
        fig, axs = plt.subplots(figsize=(25, 12.5))
    axs.set_title('Схема используемого фильтра', fontsize=38)
    axs.set_xticks([])
    axs.set_yticks([])
    axs.imshow(plt.imread(FILTER_SCHEMA_FILE))
    if axs is None:
        plt.show()


# График АЧХ передаточной функции фильтра
def amplitude_frequency_characteristic_plot(axs=None):
    if axs is None:
        fig, axs = plt.subplots(figsize=(25, 12.5))
    axs.plot(frequencies[1:size // 2], resonant_filter()[1:size // 2], c='maroon', linewidth=4)
    axs.set_title('АЧХ передаточной функции фильтра', fontsize=38)
    axs.set_xlabel('Частота, Гц', fontsize=34)
    axs.set_ylabel('Амплитуда', fontsize=34)
    tics = np.arange(0, 22500, 2500)
    tics = np.append(tics, f)
    axs.set_xticks(tics)
    axs.tick_params(axis='x', labelsize=24)
    axs.tick_params(axis='y', labelsize=30)
    axs.grid(axis='both')
    if axs is None:
        plt.show()


# Рисует каждый график в отдельности
def draw_individual_plots():
    original_signal_plot()
    original_spectrum_plot()
    amplitude_frequency_characteristic_plot()
    filtered_spectrum_plot()
    filtered_signal_plot()
    draw_filter_schema()


# Рисует сводный график
def draw_summarize_plot():
    fig, axs = plt.subplots(3, 2, figsize=(40, 20))
    axs = axs.flatten()
    original_signal_plot(axs[0])
    original_spectrum_plot(axs[1])
    filtered_signal_plot(axs[2])
    filtered_spectrum_plot(axs[3])
    amplitude_frequency_characteristic_plot(axs[4])
    draw_filter_schema(axs[5])
    plt.tight_layout()
    plt.show()


# Считывает бинарные данные из файла, преобразует их в десятичную СС
def read_and_parse_data():
    # открываем файл
    with open(ORIG_SIGNAL_FILE, 'r') as file:
        # читаем все строки файла
        lines = file.readlines()
        # переводим каждую строку из двоичной СС в десятичную, удаляя все символы, кроме 0 и 1
        original_signal = np.array(
            [int(re.sub(r'[^01]', '', line), 2) for line in lines],
            dtype=np.float64
        )
    return original_signal


# Фильтрация сигнала с помощью резонансного фильтра
def resonant_filter():
    # циклическая частота
    w = 2 * np.pi * frequencies
    # передаточная функция одного резонансного фильтра
    # замещаем NaN на 0 для безопасного использования
    return np.nan_to_num(R / (R + 1j * (w * L - 1 / (w * C))))


if __name__ == '__main__':
    # считываем исходный сигнал из файла
    original_signal = read_and_parse_data()

    # количество элементов в записи исходного сигнала
    size = original_signal.size
    # массив времени от 0 до DURATION с количеством точек равным size
    timeline = np.linspace(0, DURATION, size)
    # частота дискретизации
    fs = size / DURATION

    # получаем спектр исходного сигнала с помощью быстрого преобразования Фурье
    original_spectre = np.fft.fft(original_signal)
    # получаем частоты, соответствующие элементам спектра
    frequencies = np.fft.fftfreq(size, 1 / fs)

    # фильтруем сигнал тремя последовательно подключенными резонансными фильтрами
    filtered_spectre = original_spectre * (resonant_filter() ** 3)
    # получаем исходный сигнал из спектра с помощью обратного быстрого преобразования Фурье и берем действительную часть
    filtered_signal = np.real(np.fft.ifft(filtered_spectre))

    # рисуем отдельные графики
    draw_individual_plots()
    # рисуем обобщенный график
    draw_summarize_plot()
