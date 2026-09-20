#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>

namespace matf::verification::metamorphic_testing::relations {

std::string CapitalizationIrrelevance::mutate_input(std::string& input) {
    return input;
}
bool CapitalizationIrrelevance::holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) {
    return original == modified;
}
} // namespace matf::verification::metamorphic_testing::relations
