add_executable(test_run test.cpp)
target_link_libraries(test_run PRIVATE matf::metamorphic_testing)
target_compile_options(test_run PRIVATE -Wall -Wextra -Wpedantic)
