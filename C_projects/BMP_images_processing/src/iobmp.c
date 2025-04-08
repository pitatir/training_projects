#include "iobmp.h"

void freeImageData(RGB **image_data, size_t height) {
    for (size_t j = 0; j < height; j++) {
        free(image_data[j]);
    }
    free(image_data);
}


void freeImage(BMP *image) {
    freeImageData(image->data, image->BitmapFileInfoHeader.Height);
    free(image);
}


void freeCoordinatesArrays(CoordinatesArrays *coordinates_arrays) {
    free(coordinates_arrays->x_left);
    free(coordinates_arrays->y_left);
    free(coordinates_arrays->x_right);
    free(coordinates_arrays->y_right);
    free(coordinates_arrays);
}


size_t countBitsToSkip(BMP *image) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;
    size_t file_size = image->BitmapFileHeader.FileSize;

    size_t real_data_size = file_size - sizeof(Header) - sizeof(InfoHeader);
    size_t theory_data_size = width * height * sizeof(RGB);
    size_t bits = (real_data_size - theory_data_size) / height;

    return bits;
}


size_t countBitsToAdd(size_t width, size_t height) {
    size_t theory_data_size = width * height * sizeof(RGB);
    size_t real_data_size;
    size_t num_1;
    size_t num_2;
    size_t bits;

    if (theory_data_size % (height * 4)) {
        num_1 = theory_data_size / 4;
        num_2 = (num_1 / height + 1) * height;
        real_data_size = num_2 * 4;
        bits = (real_data_size - theory_data_size) / height;

        return bits;
    }

    return 0;
}


Error checkInfoHeader(BMP *image) {
    if (image->BitmapFileInfoHeader.Size != 40) {
        printf("It's not the 3rd version of BMP!\n");
        return ERROR_BMP_FORMAT;
    }
    if (image->BitmapFileInfoHeader.BitsPerPixel != 24) {
        printf("It's not 24 bits per pixel!\n");
        return ERROR_BMP_FORMAT;
    }
    if (image->BitmapFileInfoHeader.Compression != 0) {
        printf("It's compressed!\n");
        return ERROR_BMP_FORMAT;
    }
    return NO_ERROR;
}


BMP *openImage(const char *path) {
    char *format;
    BMP *image;
    size_t extra_bits;
    size_t width;
    size_t height;
    size_t read;

    FILE *file = fopen(path, "rb");
    if (!file) {
        some_kind_of_image_error = ERROR_OPEN_FILE;
        return NULL;
    }

    format = strrchr(path, '.');
    if (!(format && strcmp(format, ".bmp") == 0)) {
        fclose(file);
        some_kind_of_image_error = ERROR_FILE_FORMAT;
        return NULL;
    }

    image = (BMP *) malloc(sizeof(BMP));
    if (!image) {
        fclose(file);
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    rewind(file);

    read = fread(&image->BitmapFileHeader, sizeof(Header), 1, file);
    if (read != 1) {
        fclose(file);
        free(image);
        some_kind_of_image_error = ERROR_READ_FILE;
        return NULL;
    }
    if (image->BitmapFileHeader.Signature != 0x4D42) {
        fclose(file);
        free(image);
        some_kind_of_image_error = ERROR_FILE_FORMAT;
        return NULL;
    }

    read = fread(&image->BitmapFileInfoHeader, sizeof(InfoHeader), 1, file);
    if (read != 1) {
        fclose(file);
        free(image);
        some_kind_of_image_error = ERROR_READ_FILE;
        return NULL;
    }
    if (checkInfoHeader(image)) {
        fclose(file);
        free(image);
        some_kind_of_image_error = ERROR_BMP_FORMAT;
        return NULL;
    }

    read = 0;
    width = image->BitmapFileInfoHeader.Width;
    height = image->BitmapFileInfoHeader.Height;
    extra_bits = countBitsToSkip(image);

    image->data = (RGB **) malloc(sizeof(RGB *) * height);
    if (!image->data) {
        fclose(file);
        free(image);
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    for (size_t j = 0; j < height; j++) {
        image->data[j] = (RGB *) malloc(sizeof(RGB) * width + extra_bits);
        if (!image->data[j]) {
            fclose(file);
            for (size_t k = 0; k < j; k++) {
                free(image->data[k]);
            }
            free(image->data);
            free(image);
            some_kind_of_image_error = ERROR_MEMORY;
            return NULL;
        }
        read += fread(image->data[j], sizeof(RGB) * width + extra_bits, 1, file);
    }
    if (read != height) {
        fclose(file);
        freeImage(image);
        some_kind_of_image_error = ERROR_READ_FILE;
        return NULL;
    }

    fclose(file);

    return image;
}


Error saveImage(BMP *image, const char *path) {
    char *format;
    size_t extra_bits;
    size_t width;
    size_t height;
    size_t write;

    FILE *file = fopen(path, "wb");
    if (!file) {
        return ERROR_OPEN_FILE;
    }

    format = strrchr(path, '.');
    if (!(format && strcmp(format, ".bmp") == 0)) {
        fclose(file);
        remove(path);
        return ERROR_FILE_FORMAT;
    }

    write = fwrite(&image->BitmapFileHeader, sizeof(Header), 1, file);
    if (write != 1) {
        fclose(file);
        remove(path);
        return ERROR_WRITE_FILE;
    }
    write = fwrite(&image->BitmapFileInfoHeader, sizeof(InfoHeader), 1, file);
    if (write != 1) {
        fclose(file);
        remove(path);
        return ERROR_WRITE_FILE;
    }

    write = 0;
    width = image->BitmapFileInfoHeader.Width;
    height = image->BitmapFileInfoHeader.Height;
    extra_bits = countBitsToSkip(image);

    for (size_t j = 0; j < height; j++) {
        write += fwrite(image->data[j], sizeof(RGB) * width + extra_bits, 1, file);
    }
    if (write != height) {
        fclose(file);
        remove(path);
        return ERROR_WRITE_FILE;
    }

    fclose(file);
    return NO_ERROR;
}


BMP *generateImage(RGB **buffer, size_t width, size_t height) {
    size_t extra_bits = countBitsToAdd(width, height);
    BMP *image = (BMP *) malloc(sizeof(BMP));
    if (!image) {
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    image->BitmapFileHeader.Signature = 0x4D42;
    image->BitmapFileHeader.FileSize =
            sizeof(Header) + sizeof(InfoHeader) + sizeof(RGB) * width * height + extra_bits * height;
    image->BitmapFileHeader.reserved = 0;
    image->BitmapFileHeader.DataOffset = sizeof(Header) + sizeof(InfoHeader);

    image->BitmapFileInfoHeader.Size = 40;
    image->BitmapFileInfoHeader.Width = width;
    image->BitmapFileInfoHeader.Height = height;
    image->BitmapFileInfoHeader.Planes = 1;
    image->BitmapFileInfoHeader.BitsPerPixel = 24;
    image->BitmapFileInfoHeader.Compression = 0;
    image->BitmapFileInfoHeader.ImageSize = sizeof(RGB) * width * height + extra_bits * height; // 0
    image->BitmapFileInfoHeader.XpixelsPerM = 11811; // 300dpi // 4700 ???
    image->BitmapFileInfoHeader.YpixelsPerM = 11811; // 300dpi // 4700 ???
    image->BitmapFileInfoHeader.ColorsUsed = 0; // sizeof(char) ???
    image->BitmapFileInfoHeader.ImportantColors = 0; // sizeof(char) ???

    image->data = (RGB **) malloc(sizeof(RGB *) * height);
    if (!image->data) {
        free(image);
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    for (size_t j = 0; j < height; j++) {
        image->data[j] = (RGB *) calloc(1, sizeof(RGB) * width + extra_bits);
        if (!image->data[j]) {
            for (size_t k = 0; k < j; k++) {
                free(image->data[k]);
            }
            free(image->data);
            free(image);
            some_kind_of_image_error = ERROR_MEMORY;
            return NULL;
        }
        memcpy(image->data[j], buffer[j], width * sizeof(RGB));
    }

    return image;
}


BMP *generatePlainColorImage(RGB color, size_t width, size_t height) {
    size_t extra_bits = countBitsToAdd(width, height);
    BMP *image = (BMP *) malloc(sizeof(BMP));
    if (!image) {
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    image->BitmapFileHeader.Signature = 0x4D42;
    image->BitmapFileHeader.FileSize =
            sizeof(Header) + sizeof(InfoHeader) + sizeof(RGB) * width * height + extra_bits * height;
    image->BitmapFileHeader.reserved = 0;
    image->BitmapFileHeader.DataOffset = sizeof(Header) + sizeof(InfoHeader);

    image->BitmapFileInfoHeader.Size = 40;
    image->BitmapFileInfoHeader.Width = width;
    image->BitmapFileInfoHeader.Height = height;
    image->BitmapFileInfoHeader.Planes = 1;
    image->BitmapFileInfoHeader.BitsPerPixel = 24;
    image->BitmapFileInfoHeader.Compression = 0;
    image->BitmapFileInfoHeader.ImageSize = sizeof(RGB) * width * height + extra_bits * height; // 0
    image->BitmapFileInfoHeader.XpixelsPerM = 11811; // 300dpi // 4700 ???
    image->BitmapFileInfoHeader.YpixelsPerM = 11811; // 300dpi // 4700 ???
    image->BitmapFileInfoHeader.ColorsUsed = 0; // sizeof(char) ???
    image->BitmapFileInfoHeader.ImportantColors = 0; // sizeof(char) ???

    image->data = (RGB **) malloc(sizeof(RGB *) * height);
    if (!image->data) {
        free(image);
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }
    for (size_t j = 0; j < height; j++) {
        image->data[j] = (RGB *) calloc(1, sizeof(RGB) * width + extra_bits);
        if (!image->data[j]) {
            for (size_t k = 0; k < j; k++) {
                free(image->data[k]);
            }
            free(image->data);
            free(image);
            some_kind_of_image_error = ERROR_MEMORY;
            return NULL;
        }
        for (size_t i = 0; i < width; i++) {
            image->data[j][i].Red = color.Red;
            image->data[j][i].Green = color.Green;
            image->data[j][i].Blue = color.Blue;
        }
    }

    return image;
}
