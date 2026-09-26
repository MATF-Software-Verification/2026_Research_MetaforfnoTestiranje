find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(qpdf REQUIRED)
find_package(b64 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_library(metamorphic_testing_core
    src/clients/dummy_search_client.cpp
    src/relations/capitalization_irrelevance.cpp
    src/relations/duplicate_term_irrelevance.cpp
    src/relations/term_addition_monotonicity.cpp
    src/relations/whitespace_punctuation_irrelevance.cpp
    src/relations/multiple_term_reduction.cpp
    src/relations/input_permutation.cpp
    src/relations/invalid_term_irrelevance.cpp
    src/relations/invalid_term_relevance.cpp
    src/token_generator.cpp
)
add_library(matf::metamorphic_testing_core ALIAS metamorphic_testing_core)

target_include_directories(metamorphic_testing_core PUBLIC include)
target_compile_features(metamorphic_testing_core PUBLIC cxx_std_20)
target_compile_options(metamorphic_testing_core PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(metamorphic_testing_core PUBLIC fmt::fmt spdlog::spdlog)

# Adapters to real systems: the Elasticsearch client and PDF splitting.
add_library(metamorphic_testing
    src/clients/elasticsearch_search_client.cpp
    src/pdf/pdf_splitter.cpp
)
add_library(matf::metamorphic_testing ALIAS metamorphic_testing)

target_compile_options(metamorphic_testing PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(metamorphic_testing PUBLIC matf::metamorphic_testing_core)
target_link_libraries(metamorphic_testing PRIVATE httplib::httplib nlohmann_json::nlohmann_json qpdf::libqpdf b64::b64)
