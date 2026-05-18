cmake_minimum_required(VERSION 3.10)

project( LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS " Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    Truss1DData.cpp
    Truss1DDispFieldData.cpp
    Truss1D.cpp
)

set(HEADERS
    Truss1DData.h
    Truss1DDispFieldData.h
    Truss1D.h
)

# 创建可执行文件
add_executable(Truss1D ${SOURCES} ${HEADERS})

# 链接SIFEG库
target_link_libraries(Truss1D PRIVATE SIFEG)

# 添加包含目录
target_include_directories(Truss1D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)