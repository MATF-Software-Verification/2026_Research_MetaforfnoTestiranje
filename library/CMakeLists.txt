find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(qpdf REQUIRED)
find_package(b64 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_library(metamorphic_testing
    src/clients/dummy_search_client.cpp
    src/clients/elasticsearch_search_client.cpp
    src/pdf/pdf_splitter.cpp
    src/relations/capitalization_irrelevance.cpp
    src/token_generator.cpp
)
add_library(matf::metamorphic_testing ALIAS metamorphic_testing)

target_include_directories(metamorphic_testing PUBLIC include)
target_compile_features(metamorphic_testing PUBLIC cxx_std_20)
target_compile_options(metamorphic_testing PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(metamorphic_testing PUBLIC fmt::fmt spdlog::spdlog)
target_link_libraries(metamorphic_testing PRIVATE httplib::httplib nlohmann_json::nlohmann_json qpdf::libqpdf b64::b64)
