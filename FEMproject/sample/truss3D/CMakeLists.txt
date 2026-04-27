cmake_minimum_required(VERSION 3.10)

project( LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS " Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    Truss3DData.cpp
    Truss3DDispFieldData.cpp
    Truss3D.cpp
)

set(HEADERS
    Truss3DData.h
    Truss3DDispFieldData.h
    Truss3D.h
)

# 创建可执行文件
add_executable(Truss3D ${SOURCES} ${HEADERS})

# 链接CDFEG库
target_link_libraries(Truss3D PRIVATE CDFEG)

# 添加包含目录
target_include_directories(Truss3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)
