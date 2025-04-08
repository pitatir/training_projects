#ifndef IOBMP_H
#define IOBMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "structs.h"

void freeImageData(RGB **image_data, size_t height);

void freeImage(BMP *image);

void freeCoordinatesArrays(CoordinatesArrays *coordinates_arrays);

size_t countBitsToSkip(BMP *image);

size_t countBitsToAdd(size_t width, size_t height);

Error checkInfoHeader(BMP *image);

BMP *openImage(const char *path);

Error saveImage(BMP *image, const char *path);

BMP *generateImage(RGB **buffer, size_t width, size_t height);

BMP *generatePlainColorImage(RGB color, size_t width, size_t height);


#endif
