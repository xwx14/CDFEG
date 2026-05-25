cmake_minimum_required(VERSION 3.10)

project(El2D LANGUAGES CXX)

set(SOURCES
    elMain.cpp
    elData.cpp
    ElDispFieldData.cpp
    ElQ4g.cpp
    StressBL2g.cpp
)

set(HEADERS
    elData.h
    ElDispFieldData.h
    ElQ4g.h
    StressBL2g.h
)

add_executable(el ${SOURCES} ${HEADERS})

target_link_libraries(el PRIVATE CDFEG)

target_include_directories(el PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)
