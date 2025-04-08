#include "functions.h"

void help() {
    printf("This program can change BMP images\n");
    printf("This program supports only 3rd version of BMP, 24 bits per pixel and without compression!\n");
    printf("\n");
    printf("You can choose one option from the list at a time:\n");
    printf("1) Get information about image:\n");
    printf("\t Long command format: --info --input /mnt/input_path\n");
    printf("\t Short command format: -I -i /mnt/input_path\n");
    printf("\n");
    printf("2) Reflect image area:\n");
    printf("\t Long command format: --reflect --input /mnt/input_path --axis horizontal --upper_left x:y --lower_right x:y\n");
    printf("\t Short command format: -R -i /mnt/input_path -a h -l x:y -r x:y\n");
    printf("\t Note: --axis can be horizontal (h) or vertical (v) \n");
    printf("\n");
    printf("3) Copy and paste image area:\n");
    printf("\t Long command format: --copypaste --input /mnt/input_path --upper_left x:y --lower_right x:y --paste x:y\n");
    printf("\t Short command format: -P -i /mnt/input_path -l x:y -r x:y -p x:y\n");
    printf("\n");
    printf("4) Change image color:\n");
    printf("\t Long command format: --color --input /mnt/input_path --old_color R:G:B --new_color R:G:B\n");
    printf("\t Short command format: -C -i /mnt/input_path -c R:G:B -n R:G:B\n");
    printf("\n");
    printf("5) Split image:\n");
    printf("\t Long command format: --split --input /mnt/input_path --x_parts x --y_parts y --output /mnt/output_path\n");
    printf("\t Short command format: -S -i /mnt/input_path -x x -y y -o /mnt/output_path\n");
    printf("\n");
    printf("6) Get help and see this text again:\n");
    printf("\t Long command format: --help\n");
    printf("\t Short command format: -H\n");
    printf("\n");

    printf("If you don't want to change the input image by using options 2)-4), you can add option --output /mnt/output_path or -o /mnt/output_path\n");
    printf("New image will be generated as a result of function work and saved by this path\n");
}


const char *getName(const char *path) {
    char *slash = strrchr(path, '/');
    char *name = (char *) malloc((strlen(slash) - 4) * sizeof(char));
    if (!name) {
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }
    memcpy(name, slash + 1, (strlen(slash) - 5) * sizeof(char));
    name[strlen(slash) - 5] = '\0';
    return name;
}


double getSize(BMP *image, int *flag_Kb, int *flag_Mb) {
    uint32_t file_size = image->BitmapFileHeader.FileSize;
    double file_size_2 = 0;
    if (file_size / 1024.0 > 1) {
        file_size_2 = file_size / 1024.0;
        *flag_Kb = 1;
        if (file_size_2 / 1024.0 > 1) {
            file_size_2 /= 1024.0;
            *flag_Mb = 1;
        }
    }
    return file_size_2;
}


Error info(BMP *image, const char *path) {
    int flag_Kb = 0;
    int flag_Mb = 0;
    double file_size_2;
    const char *name = getName(path);
    if (!name) {
        return some_kind_of_image_error;
    }

    printf("Information about image %s\n", path);
    printf("\tImage name: %s\n", name);
    printf("\tImage size: %u bytes", image->BitmapFileHeader.FileSize);
    file_size_2 = getSize(image, &flag_Kb, &flag_Mb);
    if (flag_Mb) {
        printf(" = %.2f Mb\n", file_size_2);
    }
    if (flag_Kb && !flag_Mb) {
        printf(" = %.2f Kb\n", file_size_2);
    }
    if (!flag_Kb && !flag_Mb) {
        printf("\n");
    }
    printf("\tImage width: %u pixels\n", image->BitmapFileInfoHeader.Width);
    printf("\tImage height: %u pixels\n", image->BitmapFileInfoHeader.Height);
    printf("\tImage bits per pixel: %u\n", image->BitmapFileInfoHeader.BitsPerPixel);
    printf("\tImage version of BMP: 3\n");
    printf("\tImage is not compressed\n");

    free((char *) name);
    return NO_ERROR;
}


int RGBcmp(RGB elem1, RGB elem2) {
    return (elem1.Red == elem2.Red) && (elem1.Green == elem2.Green) && (elem1.Blue == elem2.Blue);
}


void changeImageColor(BMP *image, RGB color, RGB new_color) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;
    int flag = 0;

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            if (RGBcmp(image->data[j][i], color)) {
                flag += flag == 0;
                image->data[j][i].Red = new_color.Red;
                image->data[j][i].Green = new_color.Green;
                image->data[j][i].Blue = new_color.Blue;
            }
        }
    }

    if (!flag) {
        printf("Color not found\n");
    }
}


int checkCopyCoordinates(BMP *image, Coordinates *left, Coordinates *right) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;

    if (left->x >= width || right->y >= height) {
        return 0;
    }
    if (right->x >= width) {
        right->x = width - 1;
    }
    if (left->y >= height) {
        left->y = height - 1;
    }
    if (left->x > right->x || right->y > left->y) {
        return 0;
    }

    return 1;
}


int
checkPasteCoordinates(BMP *image, Coordinates *paste_left, Coordinates copy_left, Coordinates copy_right,
                      size_t *start_y) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;
    size_t delta_y = copy_left.y - copy_right.y + 1;
    int paste_right_y;

    if (paste_left->x >= width) {
        return 0;
    }
    if (paste_left->y >= height) {
        paste_left->y = height - 1;
    }
    paste_right_y = (int) (paste_left->y - delta_y + 1);
    if (paste_right_y < 0) {
        paste_right_y = 0;
    }
    *start_y = paste_right_y;

    return 1;
}


ImageData *copyImageArea(BMP *image, Coordinates left, Coordinates right) {
    ImageData *image_data;
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;
    size_t delta_x = right.x - left.x + 1;
    size_t delta_y = left.y - right.y + 1;
    size_t u = 0;
    size_t v = 0;

    RGB **buffer = (RGB **) malloc(sizeof(RGB *) * delta_y);
    if (!buffer) {
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    for (size_t j = right.y; (j < left.y + 1) && (j < height); j++) {
        buffer[v] = (RGB *) malloc(sizeof(RGB) * delta_x);
        if (!buffer[v]) {
            freeImageData(buffer, v);
            some_kind_of_image_error = ERROR_MEMORY;
            return NULL;
        }
        for (size_t i = left.x; (i < right.x + 1) && (i < width); i++) {
            buffer[v][u].Red = image->data[j][i].Red;
            buffer[v][u].Green = image->data[j][i].Green;
            buffer[v][u].Blue = image->data[j][i].Blue;
            u++;
        }
        u = 0;
        v++;
    }

    image_data = (ImageData *) malloc(sizeof(ImageData));
    if (!image_data) {
        freeImageData(buffer, v);
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }
    image_data->width = delta_x;
    image_data->height = delta_y;
    image_data->data = buffer;

    return image_data;
}


void pasteImageArea(BMP *image, ImageData *buffer, Coordinates left, size_t right_y) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;
    size_t delta_x = buffer->width;
    size_t delta_y = buffer->height;
    size_t u = 0;
    size_t v = 0;

    for (size_t j = right_y; (j < left.y + 1) && (j < height) && (v < delta_y + 1); j++) {
        for (size_t i = left.x; (i < left.x + delta_x) && (i < width); i++) {
            image->data[j][i].Red = buffer->data[v][u].Red;
            image->data[j][i].Green = buffer->data[v][u].Green;
            image->data[j][i].Blue = buffer->data[v][u].Blue;
            u++;
        }
        u = 0;
        v++;
    }
}


Error copypasteImageArea(BMP *image, Coordinates *copy_left, Coordinates *copy_right, Coordinates *paste_left) {
    ImageData *buffer_data;
    size_t right_y;

    if (checkCopyCoordinates(image, copy_left, copy_right)) {
        if (checkPasteCoordinates(image, paste_left, *copy_left, *copy_right, &right_y)) {
            buffer_data = copyImageArea(image, *copy_left, *copy_right);
            if (!buffer_data) {
                return some_kind_of_image_error;
            }
            //saveImage(buffer_image, "C:\\Users\\Admin\\CLionProjects\\cw_bmp\\images\\generated\\Bridge_copied.bmp");
            pasteImageArea(image, buffer_data, *paste_left, right_y);
            freeImageData(buffer_data->data, buffer_data->height);
            free(buffer_data);
            return NO_ERROR;
        }
    }

    printf("Wrong coordinates!\n");
    return NO_ERROR;
}

const char *makePath(const char *path, int part_image_num) {
    const char *new_path;
    size_t new_path_len;
    char num[9];

    new_path_len = strlen(path) + 1 + strlen(num) + 4 + 1;
    new_path = (const char *) malloc(new_path_len * sizeof(char));
    if (!new_path) {
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    sprintf((char *) new_path, "%s/%d.bmp", path, part_image_num);

    return new_path;
}


int checkNM(BMP *image, size_t N, size_t M) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;

    return !(N > width || M > height) && !(N == 0 || M == 0);
}


// N = X, M = Y
CoordinatesArrays *countCoordinates(BMP *image, size_t N, size_t M) {
    size_t width = image->BitmapFileInfoHeader.Width;
    size_t height = image->BitmapFileInfoHeader.Height;
    size_t X_part = width / N;
    size_t Y_part = height / M;
    size_t X_remains = width % N;
    size_t Y_remains = height % M;
    size_t prev = 0;

    CoordinatesArrays *result = (CoordinatesArrays *) malloc(sizeof(CoordinatesArrays));
    if (!result) {
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    size_t *X_left = (size_t *) calloc(N, sizeof(size_t));
    size_t *Y_left = (size_t *) calloc(M, sizeof(size_t));
    size_t *X_right = (size_t *) calloc(N, sizeof(size_t));
    size_t *Y_right = (size_t *) calloc(M, sizeof(size_t));
    if (!(X_left && Y_left && X_right && Y_right)) {
        free(result);
        some_kind_of_image_error = ERROR_MEMORY;
        return NULL;
    }

    for (size_t i = 1; i < N; i++) {
        X_left[i] = prev + X_part + 1 * (X_remains != 0);
        prev = X_left[i];
        X_remains -= X_remains != 0;
    }

    prev = 0;
    for (size_t j = 1; j < M; j++) {
        Y_right[j] = prev + Y_part + 1 * (Y_remains != 0);
        prev = Y_right[j];
        Y_remains -= Y_remains != 0;
    }

    for (size_t i = 0; i < N - 1; i++) {
        X_right[i] = X_left[i + 1] - 1;
    }

    for (size_t j = 0; j < M - 1; j++) {
        Y_left[j] = Y_right[j + 1] - 1;
    }

    X_right[N - 1] = width - 1;
    Y_left[M - 1] = height - 1;

    result->x_left = X_left;
    result->y_left = Y_left;
    result->x_right = X_right;
    result->y_right = Y_right;

    return result;
}


Error splitImage(BMP *image, const char *path, size_t N, size_t M) {
    Error result_error;
    CoordinatesArrays *coordinates_arrays;
    Coordinates coordinates_pair_left;
    Coordinates coordinates_pair_right;
    ImageData *image_part_data;
    BMP *image_part;
    const char *image_part_path;
    int num = 1;

    if (checkNM(image, N, M)) {
        coordinates_arrays = countCoordinates(image, N, M);
        if (!coordinates_arrays) {
            return some_kind_of_image_error;
        }
        for (size_t j = 0; j < M; j++) {
            for (size_t i = 0; i < N; i++) {
                coordinates_pair_left.x = coordinates_arrays->x_left[i];
                coordinates_pair_left.y = coordinates_arrays->y_left[j];
                coordinates_pair_right.x = coordinates_arrays->x_right[i];
                coordinates_pair_right.y = coordinates_arrays->y_right[j];

                image_part_data = copyImageArea(image, coordinates_pair_left, coordinates_pair_right);
                if (!image_part_data) {
                    freeCoordinatesArrays(coordinates_arrays);
                    return some_kind_of_image_error;
                }
                image_part = generateImage(image_part_data->data, image_part_data->width, image_part_data->height);
                if (!image_part) {
                    freeCoordinatesArrays(coordinates_arrays);
                    freeImageData(image_part_data->data, image_part_data->height);
                    free(image_part_data);
                    return some_kind_of_image_error;
                }
                image_part_path = makePath(path, num);
                if (!image_part_path) {
                    freeCoordinatesArrays(coordinates_arrays);
                    freeImageData(image_part_data->data, image_part_data->height);
                    free(image_part_data);
                    freeImage(image_part);
                    return some_kind_of_image_error;
                }
                result_error = saveImage(image_part, image_part_path);
                if (result_error) {
                    freeCoordinatesArrays(coordinates_arrays);
                    freeImageData(image_part_data->data, image_part_data->height);
                    free(image_part_data);
                    freeImage(image_part);
                    free((char *) image_part_path);
                    return result_error;
                }

                freeImageData(image_part_data->data, image_part_data->height);
                free(image_part_data);
                freeImage(image_part);
                free((char *) image_part_path);
                num++;
            }
        }

        freeCoordinatesArrays(coordinates_arrays);

    } else {
        printf("Image can't be split!\n");
    }
    return NO_ERROR;
}


void reflectImageVertically(ImageData *image_data) {
    size_t width = image_data->width;
    size_t height = image_data->height;
    RGB buffer;

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width / 2; i++) {
            buffer = image_data->data[j][i];
            image_data->data[j][i] = image_data->data[j][width - 1 - i];
            image_data->data[j][width - 1 - i] = buffer;
        }
    }
}


Error reflectImageHorizontally(ImageData *image_data) {
    size_t width = image_data->width;
    size_t height = image_data->height;
    RGB *buffer = (RGB *) malloc(width * sizeof(RGB));
    if (!buffer) {
        return ERROR_MEMORY;
    }

    for (size_t j = 0; j < height / 2; j++) {
        memcpy(buffer, image_data->data[j], width * sizeof(RGB));
        memcpy(image_data->data[j], image_data->data[height - 1 - j], width * sizeof(RGB));
        memcpy(image_data->data[height - 1 - j], buffer, width * sizeof(RGB));
    }

    free(buffer);
    return NO_ERROR;
}


Error reflectImageArea(BMP *image, const char *axis, Coordinates *left, Coordinates *right) {
    int is_horizontal = strcmp(axis, "horizontal") == 0 || strcmp(axis, "h") == 0;
    int is_vertical = strcmp(axis, "vertical") == 0 || strcmp(axis, "v") == 0;
    ImageData *buffer_image_data;
    Error error_result;

    if (!is_horizontal && !is_vertical) {
        printf("Wrong axis!\n");
        return NO_ERROR;
    }

    if (checkCopyCoordinates(image, left, right)) {
        buffer_image_data = copyImageArea(image, *left, *right);
        if (!buffer_image_data) {
            return some_kind_of_image_error;
        }
        if (is_horizontal) {
            error_result = reflectImageHorizontally(buffer_image_data);
            if (error_result) {
                freeImageData(buffer_image_data->data, buffer_image_data->height);
                return error_result;
            }
        } else {
            reflectImageVertically(buffer_image_data);
        }
        pasteImageArea(image, buffer_image_data, *left, right->y);
        freeImageData(buffer_image_data->data, buffer_image_data->height);
    } else {
        printf("Wrong coordinates!\n");
    }
    return NO_ERROR;
}