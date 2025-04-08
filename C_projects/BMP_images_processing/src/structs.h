#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include <stdlib.h>
#include "errors.h"

#pragma pack (push, 1)

typedef struct {
    uint16_t Signature; // Тип файла (= 0x4D42 = BM)
    uint32_t FileSize; // Размер файла в байтах
    uint32_t reserved; // (= 0)
    uint32_t DataOffset; // Смещение к началу ихображения в байтах
} Header; // 14 байт

typedef struct {
    uint32_t Size; // Размер структуры InfoHeader в байтах (= 40)
    uint32_t Width; // Ширина в пикселях
    uint32_t Height; // Высота в пикселях
    uint16_t Planes; // Количество плоскостей (= 1)
    uint16_t BitsPerPixel; // Количество битов на один пиксель (= 24)
    uint32_t Compression; // Метод сжатия (= 0 = BI_RGB)
    uint32_t ImageSize; // Размер изображения в байтах (может быть = 0)
    uint32_t XpixelsPerM; // Количество пикселей на метр по горизонтали (может быть = 0 ???)
    uint32_t YpixelsPerM; // Количество пикселей на метр по вертикали (может быть = 0 ???)
    uint32_t ColorsUsed; // Размер таблицы цветов (может быть = 0 для максимально допустимой глубины цвета)
    uint32_t ImportantColors; // Количество цветов, необходимое для отображения файла без искажений (0 = все)
} InfoHeader; // 40 байт

typedef struct {
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
} RGB;

typedef struct {
    Header BitmapFileHeader;
    InfoHeader BitmapFileInfoHeader;
    RGB **data;
} BMP;

#pragma pack (pop)

typedef struct {
    uint32_t width;
    uint32_t height;
    RGB **data;
} ImageData;

typedef struct {
    size_t x;
    size_t y;
} Coordinates;

typedef struct {
    size_t *x_left;
    size_t *y_left;
    size_t *x_right;
    size_t *y_right;
} CoordinatesArrays;

#endif
