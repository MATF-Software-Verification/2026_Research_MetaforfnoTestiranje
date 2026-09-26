#include <matf/verification/metamorphic_testing/relations/term_addition_monotonicity.hpp>

#include <algorithm>

namespace matf::verification::metamorphic_testing::relations {

std::string TermAdditionMonotonicity::mutate_input(std::string input) {
    return input + " " + token_generator.get_random_token();
}

bool TermAdditionMonotonicity::holds(const std::unordered_set<int>& original, const std::unordered_set<int>& modified) {
    return std::all_of(original.begin(), original.end(), [&modified](int id) { return modified.contains(id); });
}
} // namespace matf::verification::metamorphic_testing::relations
