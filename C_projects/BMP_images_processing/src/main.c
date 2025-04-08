#include <getopt.h>
#include "errors.h"
#include "structs.h"
#include "iobmp.h"
#include "functions.h"

char *err_desc[ERROR_COUNT] = {"No fatal errors occurred\n", "It's not a BMP file\n",
                               "This BMP format isn't supported\n",
                               "File can't be opened\n",
                               "An error occurred during reading the file\n",
                               "An error occurred during writing the file\n",
                               "Not enough memory\n", "Wrong format of getopt input\n"};

Error some_kind_of_image_error = NO_ERROR;


void safeInputErrorExit(const char *input_path, const char *output_path, char *axis) {
    if (axis) {
        free(axis);
    }
    if (input_path) {
        free((char *) input_path);
    }
    if (output_path) {
        free((char *) output_path);
    }
    fputs(err_desc[ERROR_INPUT], stderr);
    exit(ERROR_INPUT);
}


void safeFuncErrorExit(Error error, const char *input_path, const char *output_path, char *axis) {
    if (axis) {
        free(axis);
    }
    free((char *) input_path);
    if (output_path) {
        free((char *) output_path);
    }
    fputs(err_desc[error], stderr);
    exit(error);
}


int main(int argc, char *argv[]) {
    Error result_error;
    BMP *image = NULL;

    if (argc == 1) {
        help();
        return 0;
    }

    int opt;
    int opt_index = 0;
    int func = '0';
    const char *input_path = NULL;
    const char *output_path = NULL;
    char *axis = NULL;
    Coordinates upper_left;
    int flag_upper_left = 0;
    Coordinates lower_right;
    int flag_lower_right = 0;
    Coordinates paste;
    int flag_paste = 0;
    RGB old_color;
    int flag_old_color = 0;
    RGB new_color;
    int flag_new_color = 0;
    size_t x_parts;
    int flag_x_parts = 0;
    size_t y_parts;
    int flag_y_parts = 0;

    static struct option long_options[] = {
            {"help",        0, 0, 'H'},

            {"info",        0, 0, 'I'},

            {"reflect",     0, 0, 'R'},
            {"axis",        1, 0, 'a'},
            {"upper_left",  1, 0, 'l'},
            {"lower_right", 1, 0, 'r'},

            {"copypaste",   0, 0, 'P'},
            {"paste",       1, 0, 'p'},

            {"color",       0, 0, 'C'},
            {"old_color",   1, 0, 'c'},
            {"new_color",   1, 0, 'n'},

            {"split",       0, 0, 'S'},
            {"x_parts",     1, 0, 'x'},
            {"y_parts",     1, 0, 'y'},

            {"input",       1, 0, 'i'},
            {"output",      1, 0, 'o'},

            {0,             0, 0, 0}
    };


    while ((opt = getopt_long(argc, argv, "HIRa:l:r:Pp:Cc:n:Sx:y:i:o:", long_options, &opt_index)) != -1) {
        printf("opt = %c\n", opt);
        switch (opt) {
            case 'H':
                help();
                return 0;
            case 'I':
                if (func != '0') {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                func = 'I';
                break;
            case 'R':
                if (func != '0') {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                func = 'R';
                break;
            case 'P':
                if (func != '0') {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                func = 'P';
                break;
            case 'C':
                if (func != '0') {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                func = 'C';
                break;
            case 'S':
                if (func != '0') {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                func = 'S';
                break;
            case 'i':
                if (func != '0' && input_path == NULL) {
                    input_path = (const char *) malloc((strlen(optarg) + 1) * sizeof(char));
                    if (!input_path) {
                        if (output_path) {
                            free((char *) output_path);
                        }
                        fputs(err_desc[ERROR_MEMORY], stderr);
                        exit(ERROR_MEMORY);
                    }
                    strcpy((char *) input_path, optarg);
                } else {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                printf("input = %s\n", input_path);
                break;
            case 'o':
                if (func != '0' && !output_path) {
                    output_path = (const char *) malloc((strlen(optarg) + 1) * sizeof(char));
                    if (!output_path) {
                        if (input_path) {
                            free((char *) input_path);
                        }
                        fputs(err_desc[ERROR_MEMORY], stderr);
                        exit(ERROR_MEMORY);
                    }
                    strcpy((char *) output_path, optarg);
                } else {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                break;
            case 'a':
                if (func == 'R' && !axis) {
                    axis = (char *) malloc(11 * sizeof(char));
                    if (!axis) {
                        safeInputErrorExit(input_path, output_path, axis);
                    }
                    strcpy((char *) axis, optarg);
                } else {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                break;
            case 'l':
                if (!((func == 'R' || func == 'P') && !flag_upper_left &&
                      sscanf(optarg, "%lu:%lu", &upper_left.x, &upper_left.y) == 2)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_upper_left++;
                break;
            case 'r':
                if (!((func == 'R' || func == 'P') && !flag_lower_right &&
                      sscanf(optarg, "%lu:%lu", &lower_right.x, &lower_right.y) == 2)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_lower_right++;
                break;
            case 'p':
                if (!(func == 'P' && !flag_paste &&
                      sscanf(optarg, "%lu:%lu", &paste.x, &paste.y) == 2)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_paste++;
                break;
            case 'c':
                if (!(func == 'C' && !flag_old_color &&
                      sscanf(optarg, "%hhu:%hhu:%hhu", &old_color.Red, &old_color.Green, &old_color.Blue) == 3)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_old_color++;
                break;
            case 'n':
                if (!(func == 'C' && !flag_new_color &&
                      sscanf(optarg, "%hhu:%hhu:%hhu", &new_color.Red, &new_color.Green, &new_color.Blue) == 3)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_new_color++;
                break;
            case 'x':
                if (!(func == 'S' && !flag_x_parts &&
                      sscanf(optarg, "%lu", &x_parts) == 1)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_x_parts++;
                break;
            case 'y':
                if (!(func == 'S' && !flag_y_parts &&
                      sscanf(optarg, "%lu", &y_parts) == 1)) {
                    safeInputErrorExit(input_path, output_path, axis);
                }
                flag_y_parts++;
                break;
            default:
                safeInputErrorExit(input_path, output_path, axis);

        }
    }

    if (func == '0') {
        printf("No functions entered!\n");
        safeInputErrorExit(input_path, output_path, axis);
    }

    if (input_path) {
        image = openImage(input_path);
        if (!image) {
            safeFuncErrorExit(some_kind_of_image_error, input_path, output_path, axis);
        }
    } else {
        printf("No image path entered!\n");
        safeInputErrorExit(input_path, output_path, axis);
    }

    switch (func) {
        case 'I':
            result_error = info(image, input_path);
            if (result_error) {
                freeImage(image);
                safeFuncErrorExit(result_error, input_path, output_path, axis);
            }
            break;
        case 'R':
            if (axis && flag_upper_left && flag_lower_right) {
                result_error = reflectImageArea(image, axis, &upper_left, &lower_right);
                if (result_error) {
                    freeImage(image);
                    safeFuncErrorExit(result_error, input_path, output_path, axis);
                }
            } else {
                printf("Too few arguments!\n");
                freeImage(image);
                safeInputErrorExit(input_path, output_path, axis);
            }
            break;
        case 'P':
            if (flag_upper_left && flag_lower_right && flag_paste) {
                result_error = copypasteImageArea(image, &upper_left, &lower_right, &paste);
                if (result_error) {
                    freeImage(image);
                    safeFuncErrorExit(result_error, input_path, output_path, axis);
                }
            } else {
                printf("Too few arguments!\n");
                freeImage(image);
                safeInputErrorExit(input_path, output_path, axis);
            }
            break;
        case 'C':
            if (flag_old_color && flag_new_color) {
                changeImageColor(image, old_color, new_color);
            } else {
                printf("Too few arguments!\n");
                freeImage(image);
                safeInputErrorExit(input_path, output_path, axis);
            }
            break;
        case 'S':
            if (output_path && flag_x_parts && flag_y_parts) {
                result_error = splitImage(image, output_path, x_parts, y_parts);
                if (result_error) {
                    freeImage(image);
                    safeFuncErrorExit(result_error, input_path, output_path, axis);
                }
            } else {
                printf("Too few arguments!\n");
                freeImage(image);
                safeInputErrorExit(input_path, output_path, axis);
            }
            break;
        default:
            break;
    }

    if (func != 'S') {
        if (output_path) {
            result_error = saveImage(image, output_path);
        } else {
            result_error = saveImage(image, input_path);
        }
        if (result_error) {
            freeImage(image);
            safeFuncErrorExit(result_error, input_path, output_path, axis);
        }
    }

    freeImage(image);
    free((char *) input_path);
    if (output_path) {
        free((char *) output_path);
    }

    return 0;
}
