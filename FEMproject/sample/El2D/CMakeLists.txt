cmake_minimum_required(VERSION 3.10)

project(El2D LANGUAGES CXX)

set(SOURCES
    elMain.cpp
    elData.cpp
    aFieldData.cpp
    a1eq4g2.cpp
    a2ll2.cpp
)

set(HEADERS
    elData.h
    aFieldData.h
    a1eq4g2.h
    a2ll2.h
)

add_executable(el ${SOURCES} ${HEADERS})

target_link_libraries(el PRIVATE CDFEG)

target_include_directories(el PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)
