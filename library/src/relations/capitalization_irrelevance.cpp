#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>

#include <algorithm>
#include <cctype>
#include <string>

namespace matf::verification::metamorphic_testing::relations {

namespace {

bool has_lower(const std::string& input) {
    return std::any_of(input.begin(), input.end(), [](unsigned char c) { return std::islower(c) != 0; });
}

} // namespace

std::string CapitalizationIrrelevance::mutate_input(std::string input) {
    const bool lower = !has_lower(input);

    std::transform(input.begin(), input.end(), input.begin(),
                   [lower](unsigned char c) { return static_cast<char>(lower ? std::tolower(c) : std::toupper(c)); });
    return input;
}
bool CapitalizationIrrelevance::holds(const std::unordered_set<int>& original,
                                      const std::unordered_set<int>& modified) {
    return original == modified;
}
} // namespace matf::verification::metamorphic_testing::relations
