cmake_minimum_required(VERSION 3.10)

project(El2D0 LANGUAGES CXX)

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

add_executable(el2d0 ${SOURCES} ${HEADERS})

target_link_libraries(el2d0 PRIVATE CDFEG)

target_include_directories(el2d0 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)
