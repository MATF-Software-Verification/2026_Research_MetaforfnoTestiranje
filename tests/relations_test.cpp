#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/duplicate_term_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/input_permutation.hpp>
#include <matf/verification/metamorphic_testing/relations/invalid_term_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/invalid_term_relevance.hpp>
#include <matf/verification/metamorphic_testing/relations/multiple_term_reduction.hpp>
#include <matf/verification/metamorphic_testing/relations/term_addition_monotonicity.hpp>
#include <matf/verification/metamorphic_testing/relations/whitespace_punctuation_irrelevance.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace matf::verification::metamorphic_testing;
using namespace matf::verification::metamorphic_testing::relations;

TEST_CASE("capitalization_irrelevance flips the case") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    CapitalizationIrrelevance relation(tokens);

    CHECK(relation.mutate_input("apple") == "APPLE");
    CHECK(relation.mutate_input("APPLE") == "apple");
    CHECK(relation.holds({1, 2}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}

TEST_CASE("whitespace_punctuation_irrelevance wraps the input") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    WhitespacePunctuationIrrelevance relation(tokens);

    const auto mutated = relation.mutate_input("apple");
    CHECK(mutated != "apple");
    CHECK(mutated.find("apple") != std::string::npos);
    CHECK(relation.holds({1, 2}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}

TEST_CASE("term_addition_monotonicity adds a known word") {
    TokenGenerator tokens({"grape"}, 0);
    TermAdditionMonotonicity relation(tokens);

    CHECK(relation.mutate_input("apple") == "apple grape");
    CHECK(relation.holds({1}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}

TEST_CASE("duplicate_term_irrelevance repeats the input") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    DuplicateTermIrrelevance relation(tokens);

    CHECK(relation.mutate_input("apple") == "apple apple");
    CHECK(relation.holds({1, 2}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1, 2, 3}));
}

TEST_CASE("multiple_term_reduction keeps only the first word") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    MultipleTermReduction relation(tokens);

    CHECK(relation.get_operator() == QueryOperator::And);
    CHECK(relation.mutate_input("apple grape") == "apple");
    CHECK(relation.holds({1}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}

TEST_CASE("input_permutation swaps the two words") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    InputPermutation relation(tokens);

    CHECK(relation.get_operator() == QueryOperator::And);
    CHECK(relation.mutate_input("apple grape") == "grape apple");
    CHECK(relation.holds({1, 2}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}

TEST_CASE("invalid_term_irrelevance adds an unknown word") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    InvalidTermIrrelevance relation(tokens);

    const auto mutated = relation.mutate_input("apple");
    CHECK(mutated.starts_with("apple "));
    CHECK(mutated != "apple apple");
    CHECK(mutated != "apple grape");
    CHECK(relation.holds({1, 2}, {1, 2}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}

TEST_CASE("invalid_term_relevance expects no results") {
    TokenGenerator tokens({"apple", "grape"}, 0);
    InvalidTermRelevance relation(tokens);

    CHECK(relation.get_operator() == QueryOperator::And);
    CHECK(relation.holds({1, 2}, {}));
    CHECK_FALSE(relation.holds({1, 2}, {1}));
}
