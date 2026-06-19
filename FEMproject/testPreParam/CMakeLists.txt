add_executable(testPreParam testPreParam.cpp)
target_link_libraries(testPreParam PRIVATE CDFEG)
target_include_directories(testPreParam PRIVATE ${CMAKE_SOURCE_DIR})
