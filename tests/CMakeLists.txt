find_package(Catch2 3 REQUIRED)

add_executable(unit_tests
    relations_test.cpp
    token_generator_test.cpp
    verifier_test.cpp
)
target_link_libraries(unit_tests PRIVATE matf::metamorphic_testing_core Catch2::Catch2WithMain)
target_compile_options(unit_tests PRIVATE -Wall -Wextra -Wpedantic)

include(Catch)
catch_discover_tests(unit_tests)
