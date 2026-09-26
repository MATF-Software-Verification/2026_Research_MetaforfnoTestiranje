#include <matf/verification/metamorphic_testing/relations/input_permutation.hpp>

namespace matf::verification::metamorphic_testing::relations {

std::string InputPermutation::generate_input() {
    return token_generator.get_random_token() + " " + token_generator.get_random_token();
}
std::string InputPermutation::mutate_input(std::string input) {
    auto pos = input.find(' ');
    if (pos != std::string::npos) {
        input = input.substr(pos + 1) + " " + input.substr(0, pos);
    }
    return input;
}

bool InputPermutation::holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) {
    return original == modified;
}

} // namespace matf::verification::metamorphic_testing::relations
