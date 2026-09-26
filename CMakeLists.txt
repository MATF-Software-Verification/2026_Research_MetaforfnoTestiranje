cmake_minimum_required(VERSION 4.2)
project(MetamorfnoTestiranje LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(METAMORPHIC_TESTING_BUILD_TESTS "Build the unit tests" ON)

add_subdirectory(library)
add_subdirectory(runner)

if(METAMORPHIC_TESTING_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
