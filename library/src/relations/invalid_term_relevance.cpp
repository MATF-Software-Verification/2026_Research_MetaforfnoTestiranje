#include <matf/verification/metamorphic_testing/relations/invalid_term_relevance.hpp>

namespace matf::verification::metamorphic_testing::relations {

std::string InvalidTermRelevance::mutate_input(std::string input) {
    return input + " " + token_generator.get_invalid_token();
}

// Under AND every term must match, and the invalid term matches no document.
bool InvalidTermRelevance::holds(const std::unordered_set<int>& /*original*/, const std::unordered_set<int>& modified) {
    return modified.empty();
}
} // namespace matf::verification::metamorphic_testing::relations
