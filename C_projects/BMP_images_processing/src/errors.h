#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    NO_ERROR = 0,
    ERROR_FILE_FORMAT = 1,
    ERROR_BMP_FORMAT = 2,
    ERROR_OPEN_FILE = 3,
    ERROR_READ_FILE = 4,
    ERROR_WRITE_FILE = 5,
    ERROR_MEMORY = 6,
    ERROR_INPUT = 7,

    ERROR_COUNT = 8,
} Error;

extern char *err_desc[ERROR_COUNT];

extern Error some_kind_of_image_error;

void safeInputErrorExit(const char *input_path, const char *output_path, char* axis);

void safeFuncErrorExit(Error error, const char *input_path, const char *output_path, char* axis);

#endif
