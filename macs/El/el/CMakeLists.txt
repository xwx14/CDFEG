cmake_minimum_required(VERSION 3.10)

project(el LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "el Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    elData.cpp
    elaFieldData.cpp
    a1eq4g2.cpp
    a2ll2.cpp
    elbFieldData.cpp
    beq4g2.cpp
)

set(HEADERS
    elData.h
    elaFieldData.h
    a1eq4g2.h
    a2ll2.h
    elbFieldData.h
    beq4g2.h
)

# 创建可执行文件
add_executable(el ${SOURCES} ${HEADERS})

# 链接CDFEG库
target_link_libraries(el PRIVATE CDFEG)

# 添加包含目录
target_include_directories(el PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)