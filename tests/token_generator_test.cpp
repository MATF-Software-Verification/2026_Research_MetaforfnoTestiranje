#include <matf/verification/metamorphic_testing/token_generator.hpp>

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

using namespace matf::verification::metamorphic_testing;

TEST_CASE("an empty token list or a missing file is rejected") {
    CHECK_THROWS_AS(TokenGenerator({}, 0), std::invalid_argument);
    CHECK_THROWS_AS(TokenGenerator::from_file("does_not_exist.txt", 0), std::runtime_error);
}

TEST_CASE("the same seed gives the same tokens") {
    TokenGenerator first({"apple", "banana", "cherry"}, 42);
    TokenGenerator second({"apple", "banana", "cherry"}, 42);

    for (int i = 0; i < 10; ++i) {
        CHECK(first.get_random_token() == second.get_random_token());
    }
}

TEST_CASE("a random token comes from the list") {
    TokenGenerator tokens({"apple", "banana"}, 0);

    for (int i = 0; i < 10; ++i) {
        const auto token = tokens.get_random_token();
        CHECK((token == "apple" || token == "banana"));
    }
}

TEST_CASE("an invalid token is not in the list") {
    TokenGenerator tokens({"apple", "banana"}, 0);

    const auto token = tokens.get_invalid_token();
    CHECK(token.size() == 12);
    CHECK(token != "apple");
    CHECK(token != "banana");
}
