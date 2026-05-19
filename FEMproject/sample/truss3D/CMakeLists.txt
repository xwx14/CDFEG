cmake_minimum_required(VERSION 3.10)

project(Truss3D LANGUAGES CXX)

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

add_executable(Truss3D ${SOURCES} ${HEADERS})

target_link_libraries(Truss3D PRIVATE CDFEG)

target_include_directories(Truss3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)
