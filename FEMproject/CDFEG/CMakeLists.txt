cmake_minimum_required(VERSION 3.10)

project(SIFEG LANGUAGES CXX)

# 使用父级CMakeLists.txt中定义的Eigen目录
message(STATUS "SIFEG Eigen include directory: ${EIGEN_INCLUDE_DIR}")

set(SOURCES
    EquationSystem.cpp
    FemData.cpp
    gidProPost.cpp
    IsoEleBase.cpp
    MatrixFun.cpp
    PhyFieldData.cpp
    Processor.cpp
    EleSub.cpp
)

set(HEADERS
    EquationSystem.h
    FemData.h
    gidProPost.h
    IsoEleBase.h
    MatrixFun.h
    PhyFieldData.h
    Processor.h
    EleSub.h
    SIFEG.h
)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
add_library(SIFEG SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(SIFEG PRIVATE SIFEG_EXPORTS)
target_include_directories(SIFEG PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
)
