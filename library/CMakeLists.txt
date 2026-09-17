add_library(metamorphic_testing
    src/clients/dummy_search_client.cpp
    src/relations/capitalization_irrelevance.cpp
)
add_library(matf::metamorphic_testing ALIAS metamorphic_testing)

target_include_directories(metamorphic_testing PUBLIC include)
target_compile_features(metamorphic_testing PUBLIC cxx_std_20)
target_compile_options(metamorphic_testing PRIVATE -Wall -Wextra -Wpedantic)
