cmake_minimum_required(VERSION 3.14)

project(theadPool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")
# 设置可执行文件输出路径
message(STATUS "EXECUTABLE_OUTPUT_PATH: ${PROJECT_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置库文件输出路径
message(STATUS "LIBRARY_OUTPUT_PATH: ${PROJECT_SOURCE_DIR}/lib")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 添加头文件路径
message(STATUS "INCLUDE_DIRECTORIES: ${PROJECT_SOURCE_DIR}/include")
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加库文件路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/src/thread_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/result_type.cpp
)
add_library(threadPool SHARED ${SRC_LIST})

target_link_libraries(threadPool pthread)


set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/test/test_start.cpp
)
add_executable(threadPool_test ${SRC_LIST})
target_link_libraries(threadPool_test threadPool pthread)
