#include <CL/cl.h>
#include <cstdio>
#include <cstdlib>
#include <random>

#define MATRIX_SIZE 64
#define BLOCK_SIZE 16 // надо менять и размер блока в строке 13!
// нельзя больше 32 на CUDA (PLATFORM_ID 0)
// нельзя больше 16 на Intel(R) HD Graphics (PLATFORM_ID 1)
// нельзя больше 64 на Intel(R) CPU (PLATFORM_ID 2)
#define PLATFORM_ID 1
#define LOG false

// Загрузка и компиляция программы
auto kernelSource = R"(
#define BLOCK_SIZE 16
__kernel
__attribute((reqd_work_group_size(BLOCK_SIZE, BLOCK_SIZE, 1))) void mmul(
__global int *restrict C, __global int *restrict A, __global int *restrict B, int M,int N, int K) {
    // локальная память для одного блока матриц А и В
    __local int A_local[BLOCK_SIZE][BLOCK_SIZE];
    __local int B_local[BLOCK_SIZE][BLOCK_SIZE];
    int block_x = get_group_id(0); // индекс блока
    int block_y = get_group_id(1);

    // локальный индекс (отступ внутри блока)
    int local_x = get_local_id(0);
    int local_y = get_local_id(1);

    // границы вычислений
    int a_start = M * BLOCK_SIZE * block_y;
    int a_end = a_start + M - 1;
    int b_start = BLOCK_SIZE * block_x;
    int running_sum = 0;

    // начать вычисление выходной матрицы С
    // каждая итерация цикла соответствует одному блоку матрицы
    for (int a = a_start, b = b_start; a <= a_end; a += BLOCK_SIZE, b += (BLOCK_SIZE * K)) {
        // копировать входные матрицы в локальную память
        A_local[local_x][local_y] = A[a + M * local_y + local_x]; B_local[local_x][local_y] = B[b + K * local_y + local_x];
        // дождаться окончания копирования
        barrier(CLK_LOCAL_MEM_FENCE);

        // вычислить один элемент матрицы С
        // осуществить полную размотку цикла
        #pragma unroll
        for(int k = 0; k < BLOCK_SIZE; ++k) {
            running_sum += A_local[k][local_y] * B_local[local_x][k];
        }

        // окончание обработки блока
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // записать результат вычислений
    C[get_global_id(1) * get_global_size(0) + get_global_id(0)] = running_sum;
})";

void fillMatrix(int *matrix, const int rows, const int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);

    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = dis(gen);
    }
}

void printMatrix(const char *name, const int *matrix, const int rows, const int cols) {
    printf("\n%s:\n", name);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

// Функция для проверки ошибок OpenCL
void checkError(const cl_int err, const char *operation) {
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error during operation '%s': %d\n", operation, err);
        exit(EXIT_FAILURE);
    }
}

int main() {
    system("chcp 65001");

    cl_uint num_of_platforms;
    cl_int err = clGetPlatformIDs(0, nullptr, &num_of_platforms);
    checkError(err, "Getting number of platforms");

    // Выбор платформы
    const auto availible_platforms = static_cast<cl_platform_id *>(malloc(num_of_platforms * sizeof(cl_platform_id)));
    err = clGetPlatformIDs(num_of_platforms, availible_platforms, nullptr);
    checkError(err, "Getting platform IDs");

    if constexpr (LOG) {
        printf("\n");
        for (cl_uint i = 0; i < num_of_platforms; ++i) {
            char platform_name[128];
            err = clGetPlatformInfo(availible_platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name,
                                    nullptr);
            checkError(err, "Getting platform name");
            printf("Платформа по индексу %d: %s\n", i, platform_name);
        }
    }

    cl_platform_id platform = availible_platforms[PLATFORM_ID];
    free(availible_platforms);

    // Выбор девайса
    cl_device_id devices[5];
    cl_uint num_of_devices;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 5, devices, &num_of_devices);
    checkError(err, "Getting platform devices");
    cl_device_id device = devices[0];

    int matrix_A_rows = MATRIX_SIZE;
    int matrix_A_cols = MATRIX_SIZE; // A_cols = B_rows
    int matrix_B_cols = MATRIX_SIZE;

    const auto matrix_A = static_cast<int *>(malloc(matrix_A_rows * matrix_A_cols * sizeof(int)));
    const auto matrix_B = static_cast<int *>(malloc(matrix_A_cols * matrix_B_cols * sizeof(int)));
    const auto matrix_C = static_cast<int *>(malloc(matrix_A_rows * matrix_B_cols * sizeof(int)));

    fillMatrix(matrix_A, matrix_A_rows, matrix_A_cols);
    fillMatrix(matrix_B, matrix_A_cols, matrix_B_cols);

    // Создание контекста
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    checkError(err, "Creating context");

    // Создание командной очереди
    cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    checkError(err, "Creating command queue");

    // Создание буферов
    cl_mem buf_matrix_A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 matrix_A_rows * matrix_A_cols * sizeof(int), matrix_A, &err);
    checkError(err, "Creating buffer A");

    cl_mem buf_matrix_B = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 matrix_A_cols * matrix_B_cols * sizeof(int), matrix_B, &err);
    checkError(err, "Creating buffer B");

    cl_mem buf_matrix_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                 matrix_A_rows * matrix_B_cols * sizeof(int), nullptr, &err);
    checkError(err, "Creating buffer C");

    // Создание программы
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err);
    checkError(err, "Creating program");

    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t build_log_len;
        cl_int errcode = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &build_log_len);
        if (errcode) {
            printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
            exit(-1);
        }

        const auto buff_erro = static_cast<char *>(malloc(build_log_len));
        if (!buff_erro) {
            printf("malloc failed at line %d\n", __LINE__);
            exit(-2);
        }

        errcode = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, build_log_len, buff_erro, nullptr);
        if (errcode) {
            printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
            exit(-3);
        }

        fprintf(stderr, "Build log: \n%s\n", buff_erro); //Be careful with  the fprint
        free(buff_erro);
        fprintf(stderr, "clBuildProgram failed\n");
        exit(EXIT_FAILURE);
    }
    checkError(err, "Building program");

    // Создание ядра
    cl_kernel kernel = clCreateKernel(program, "mmul", &err);
    checkError(err, "Creating kernel");

    // Установка аргументов ядра
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_matrix_C);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_matrix_A);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_matrix_B);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &matrix_A_rows);
    err |= clSetKernelArg(kernel, 4, sizeof(int), &matrix_B_cols);
    err |= clSetKernelArg(kernel, 5, sizeof(int), &matrix_A_cols);

    checkError(err, "Setting kernel arguments");

    // Определение размеров рабочей группы
    size_t global_size[2] = {MATRIX_SIZE, MATRIX_SIZE};
    size_t local_size[2] = {BLOCK_SIZE, BLOCK_SIZE};
    if (global_size[0] % local_size[0] != 0 || global_size[1] % local_size[1] != 0) {
        fprintf(stderr, "Размер матрицы должен делиться на размер блока!\n");
        exit(EXIT_FAILURE);
    }

    cl_event event;

    // Запуск ядра
    err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr,
                                 global_size, local_size,
                                 0, nullptr, &event);
    checkError(err, "Enqueueing kernel");

    // Чтение результата
    err = clEnqueueReadBuffer(queue, buf_matrix_C, CL_TRUE,
                              0, matrix_A_rows * matrix_B_cols * sizeof(int), matrix_C,
                              0, nullptr, nullptr);
    checkError(err, "Reading buffer C");

    err = clWaitForEvents(1, &event);
    checkError(err, "Waiting for event");

    cl_ulong start_time;
    cl_ulong end_time;

    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                  sizeof(cl_ulong), &start_time, nullptr);
    err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                   sizeof(cl_ulong), &end_time, nullptr);
    checkError(err, "Getting profiling info");

    const auto time = static_cast<double>(end_time - start_time) / 1e6; // Время в миллисекундах

    if constexpr (LOG) {
        printMatrix("Матрица А", matrix_A, matrix_A_rows, matrix_A_cols);
        printMatrix("Матрица B", matrix_B, matrix_A_cols, matrix_B_cols);
        printMatrix("Результат умножения матрицы А на матрицу Б", matrix_C, matrix_A_rows, matrix_B_cols);
    }

    char current_platform_name[128];
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(current_platform_name), current_platform_name, nullptr);
    checkError(err, "Getting current platform name");

    printf("\nБлочное умножение матриц с помощью \"OPEN CL\"\n");
    printf("Сторона матрицы    : %d \n", MATRIX_SIZE);
    printf("Размер блока       : %d \n", BLOCK_SIZE);
    printf("Платформа          : %s \n", current_platform_name);
    printf("Время умножения    : %3.3f мс\n", time);


    clReleaseMemObject(buf_matrix_A);
    clReleaseMemObject(buf_matrix_B);
    clReleaseMemObject(buf_matrix_C);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(matrix_A);
    free(matrix_B);
    free(matrix_C);

    return 0;
}
