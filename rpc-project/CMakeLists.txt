cmake_minimum_required(VERSION 3.14)

project(mprpc)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")
# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(MUDUO_PATH /usr/local/muduo)
# 添加头文件路径
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${MUDUO_PATH}/include)

# 添加库文件路径
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(${MUDUO_PATH}/lib)

add_subdirectory(src)
add_subdirectory(example)