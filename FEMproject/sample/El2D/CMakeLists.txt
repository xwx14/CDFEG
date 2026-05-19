cmake_minimum_required(VERSION 3.10)

project(Elastic2D LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "Elastic2D Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    Elastic2DData.cpp
    Elastic2DDispFieldData.cpp
    ElT3.cpp
    ElQ4.cpp
)

set(HEADERS
    Elastic2DData.h
    Elastic2DDispFieldData.h
    ElT3.h
    ElQ4.h
)

# 创建可执行文件
add_executable(Elastic2D ${SOURCES} ${HEADERS})

# 链接CDFEG库
target_link_libraries(Elastic2D PRIVATE CDFEG)

# 添加包含目录
target_include_directories(Elastic2D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)