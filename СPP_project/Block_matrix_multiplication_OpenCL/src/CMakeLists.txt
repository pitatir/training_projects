cmake_minimum_required(VERSION 3.10)
project(MatrixMulOpenCL)

set(OpenCL_INCLUDE_DIRS "C:/msys64/mingw64/include")
set(OpenCL_LIBRARIES "C:/msys64/mingw64/lib/libOpenCL.dll.a")

set(SOURCES main.cpp)

add_executable(MatrixMulOpenCL ${SOURCES})

target_include_directories(MatrixMulOpenCL PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(MatrixMulOpenCL ${OpenCL_LIBRARIES})
