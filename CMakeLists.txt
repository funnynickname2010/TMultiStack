cmake_minimum_required(VERSION 3.15)
project(t_multi_stack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Your header-only library (no .cpp => INTERFACE, not STATIC)
add_library(my_lib INTERFACE)
target_include_directories(my_lib INTERFACE ${CMAKE_SOURCE_DIR}/src)

# Enable testing
enable_testing()

# Add GoogleTest from local folder
add_subdirectory(external/googletest)  # <- this will build gtest and gmock


# Create test executable
add_executable(unit_tests
    tests/test_tmultistack.cpp
)

# Link libraries to the test executable
target_link_libraries(unit_tests
    PRIVATE my_lib
    PRIVATE gtest_main
)

# Discover tests automatically
include(GoogleTest)
add_test(NAME unit_tests COMMAND unit_tests)
