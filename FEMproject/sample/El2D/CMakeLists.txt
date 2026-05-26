cmake_minimum_required(VERSION 3.10)

project(el LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "el Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    elData.cpp
    ElDispFieldData.cpp
    ElQ4g.cpp
    ElT3g.cpp
    StressBL2g.cpp
)

set(HEADERS
    elData.h
    ElDispFieldData.h
    ElQ4g.h
    ElT3g.h
    StressBL2g.h
)

# 创建可执行文件
add_executable(el2d ${SOURCES} ${HEADERS})

# 链接CDFEG库
target_link_libraries(el2d PRIVATE CDFEG)

# 添加包含目录
target_include_directories(el2d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)