cmake_minimum_required(VERSION 3.10)

project(el3d LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "el3d Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    el3dMain.cpp
    el3dData.cpp
    aFieldData.cpp
    a1ec8g2.cpp
    a2ew4g2.cpp
    a3lq4.cpp
)

set(HEADERS
    el3dData.h
    aFieldData.h
    a1ec8g2.h
    a2ew4g2.h
    a3lq4.h
)

# 创建可执行文件
add_executable(el3d ${SOURCES} ${HEADERS})

# 链接SIFEG库
target_link_libraries(el3d PRIVATE SIFEG)

# 添加包含目录
target_include_directories(el3d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)
