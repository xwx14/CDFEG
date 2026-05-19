cmake_minimum_required(VERSION 3.10)

project(Truss1D LANGUAGES CXX)

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

add_executable(Truss1D ${SOURCES} ${HEADERS})

target_link_libraries(Truss1D PRIVATE CDFEG)

target_include_directories(Truss1D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/CDFEG
    ${CMAKE_SOURCE_DIR}/include
)
