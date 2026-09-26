#include <matf/verification/metamorphic_testing/relations/multiple_term_reduction.hpp>

namespace matf::verification::metamorphic_testing::relations {
 
    std::string MultipleTermReduction::generate_input() {
        return token_generator.get_random_token() + " " + token_generator.get_random_token();
    }
    std::string MultipleTermReduction::mutate_input(std::string input) {
        return input.substr(0, input.find(' '));
    }

    bool MultipleTermReduction::holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) {
        return std::all_of(original.begin(), original.end(), [&modified](int id) { return modified.contains(id); });
    }

}