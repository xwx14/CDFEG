cmake_minimum_required(VERSION 3.10)
project(cdfeg_unit_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Task 15 将替换 SOURCES 为 catch_main.cpp + test_*.cpp
set(SOURCES
    unit_main_placeholder.cpp
)

add_executable(cdfeg_unit_test ${SOURCES})
target_link_libraries(cdfeg_unit_test PRIVATE CDFEG)
target_include_directories(cdfeg_unit_test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${EIGEN_INCLUDE_DIR}
)
