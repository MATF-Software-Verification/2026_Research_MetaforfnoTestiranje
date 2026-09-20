#include <matf/verification/metamorphic_testing/relations/duplicate_term_irrelevance.hpp>

namespace matf::verification::metamorphic_testing::relations {

std::string DuplicateTermIrrelevance::mutate_input(std::string input) {
    return input + " " + input;
}

bool DuplicateTermIrrelevance::holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) {
    return original == modified;
}
} // namespace matf::verification::metamorphic_testing::relations
