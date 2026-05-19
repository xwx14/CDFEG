cmake_minimum_required(VERSION 3.10)

project(CDFEG LANGUAGES CXX)

set(SOURCES
    ElementBase.cpp
    EquationSystem.cpp
    FemData.cpp
    GaussEleBase.cpp
    GidPrePost2.cpp
    gidProPost.cpp
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
    GaussEleBase.h
    GidPrePost2.h
    gidProPost.h
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

set(CDFEG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/CDFEG)

add_custom_command(TARGET CDFEG POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CDFEG_INCLUDE_DIR}
    COMMENT "Copying headers to include/CDFEG"
)

foreach(HEADER ${HEADERS})
    add_custom_command(TARGET CDFEG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}
        ${CDFEG_INCLUDE_DIR}/${HEADER}
    )
endforeach()
