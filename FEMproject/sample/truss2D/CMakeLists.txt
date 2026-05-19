cmake_minimum_required(VERSION 3.10)

project(Truss2D LANGUAGES CXX)

set(SOURCES
    main.cpp
    Truss2DData.cpp
    Truss2DDispFieldData.cpp
    Truss2D.cpp
)

set(HEADERS
    Truss2DData.h
    Truss2DDispFieldData.h
    Truss2D.h
)

add_executable(Truss2D ${SOURCES} ${HEADERS})

target_link_libraries(Truss2D PRIVATE CDFEG)

target_include_directories(Truss2D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/CDFEG
    ${CMAKE_SOURCE_DIR}/include
)
