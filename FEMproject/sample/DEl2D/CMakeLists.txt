cmake_minimum_required(VERSION 3.10)

project(del2d LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "del2d Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    del2dData.cpp
    DelDispFieldData.cpp
    DelQ4g.cpp
)

set(HEADERS
    del2dData.h
    DelDispFieldData.h
    DelQ4g.h
)

# 创建可执行文件
add_executable(del2d ${SOURCES} ${HEADERS})

# 链接CDFEG库
target_link_libraries(del2d PRIVATE CDFEG)

# 添加包含目录
target_include_directories(del2d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)
