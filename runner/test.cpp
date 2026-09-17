#include <matf/verification/metamorphic_testing/clients/dummy_search_client.hpp>
#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/verifier.hpp>

namespace mt = matf::verification::metamorphic_testing;

int main() {
    mt::clients::DummySearchClient client;
    mt::relations::CapitalizationIrrelevance relation;

    mt::Verifier verifier(client);
    verifier.verify_relation("input", relation);
}
