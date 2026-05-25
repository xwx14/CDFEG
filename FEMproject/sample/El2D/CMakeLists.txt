cmake_minimum_required(VERSION 3.10)

project( LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS " Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    elMain.cpp
    elData.cpp
    aFieldData.cpp
    a1eq4g2.cpp
    a2ll2.cpp
)

set(HEADERS
    elData.h
    aFieldData.h
    a1eq4g2.h
    a2ll2.h
)

# 创建可执行文件
add_executable(el ${SOURCES} ${HEADERS})

# 链接SIFEG库
target_link_libraries(el PRIVATE SIFEG)

# 添加包含目录
target_include_directories(el PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)