#include <matf/verification/metamorphic_testing/clients/dummy_search_client.hpp>
#include <matf/verification/metamorphic_testing/relations/duplicate_term_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/invalid_term_relevance.hpp>
#include <matf/verification/metamorphic_testing/verifier.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace matf::verification::metamorphic_testing;

// DummySearchClient answers every query with document 1.

TEST_CASE("the verifier reports a relation that holds") {
    TokenGenerator tokens({"apple"}, 0);
    clients::DummySearchClient client;
    Verifier verifier(client);
    relations::DuplicateTermIrrelevance relation(tokens);

    CHECK(verifier.verify_relation(relation));
}

TEST_CASE("the verifier reports a relation that is violated") {
    TokenGenerator tokens({"apple"}, 0);
    clients::DummySearchClient client;
    Verifier verifier(client);
    relations::InvalidTermRelevance relation(tokens);

    CHECK_FALSE(verifier.verify_relation(relation));
}
