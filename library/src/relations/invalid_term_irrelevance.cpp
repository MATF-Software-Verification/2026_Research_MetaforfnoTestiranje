#include <matf/verification/metamorphic_testing/relations/invalid_term_irrelevance.hpp>

namespace matf::verification::metamorphic_testing::relations {

std::string InvalidTermIrrelevance::mutate_input(std::string input) {
    return input + " " + token_generator.get_invalid_token();
}

bool InvalidTermIrrelevance::holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) {
    return original == modified;
}
} // namespace matf::verification::metamorphic_testing::relations
