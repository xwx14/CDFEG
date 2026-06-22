add_executable(testGetCoef testGetCoef.cpp)
target_include_directories(testGetCoef PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(testGetCoef PRIVATE CDFEG)
