cmake_minimum_required(VERSION 3.10)

project(CDFEG LANGUAGES CXX)

set(SOURCES
    ElementBase.cpp
    EquationSystem.cpp
    FemData.cpp

    GidPrePost2.cpp
    GidResItem.cpp
    gidPrePost.cpp
    inpReader.cpp
    IsoEleBase.cpp
    MatrixFun.cpp
    PhyFieldData.cpp
    Processor.cpp
    TextReader.cpp
    vtkPost.cpp
)

set(HEADERS
    CDFEG.h
    ElementBase.h
    EquationSystem.h
    FemData.h

    GidPrePost2.h
    GidResItem.h
    gidPrePost.h
    InpDataStructures.h
    inpReader.h
    IsoEleBase.h
    MatrixFun.h
    PhyFieldData.h
    Processor.h
    TextReader.h
    vtkCellType.h
    vtkPost.h
)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
add_library(CDFEG SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(CDFEG PRIVATE CDFEG_EXPORTS)
target_include_directories(CDFEG PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
)
