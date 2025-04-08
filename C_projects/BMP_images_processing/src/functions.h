#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "structs.h"
#include "iobmp.h"

void help();

const char *getName(const char *path);

double getSize(BMP *image, int *flag_Kb, int *flag_Mb);

Error info(BMP *image, const char *path);

int RGBcmp(RGB elem1, RGB elem2);

void changeImageColor(BMP *image, RGB color, RGB new_color);

int checkCopyCoordinates(BMP *image, Coordinates *left, Coordinates *right);

int checkPasteCoordinates(BMP *image, Coordinates *paste_left, Coordinates copy_left, Coordinates copy_right,
                          size_t *start_y);

ImageData *copyImageArea(BMP *image, Coordinates left, Coordinates right);

void pasteImageArea(BMP *image, ImageData *buffer, Coordinates left, size_t right_y);

Error copypasteImageArea(BMP *image, Coordinates *copy_left, Coordinates *copy_right, Coordinates *paste_left);

const char *makePath(const char *path, int part_image_num);

int checkNM(BMP *image, size_t N, size_t M);

CoordinatesArrays *countCoordinates(BMP *image, size_t N, size_t M);

Error splitImage(BMP *image, const char *path, size_t N, size_t M);

void reflectImageVertically(ImageData *image_data);

Error reflectImageHorizontally(ImageData *image_data);

Error reflectImageArea(BMP *image, const char *axis, Coordinates *left, Coordinates *right);

#endif
