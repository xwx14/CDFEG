cmake_minimum_required(VERSION 3.10)

project(ElasticT3 LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "ElasticT3 Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    main.cpp
    ElasticT3Data.cpp
    Elastic2DDispFieldData.cpp
    ElT3.cpp
)

set(HEADERS
    ElasticT3Data.h
    Elastic2DDispFieldData.h
    ElT3.h
)

# 创建可执行文件
add_executable(ElasticT3 ${SOURCES} ${HEADERS})

# 链接CDFEG库
target_link_libraries(ElasticT3 PRIVATE CDFEG)

# 添加包含目录
target_include_directories(ElasticT3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)