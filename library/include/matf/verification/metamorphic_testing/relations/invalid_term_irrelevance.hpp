#pragma once
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

namespace matf::verification::metamorphic_testing::relations {

class InvalidTermIrrelevance : public MetamorphicRelation {
public:
    explicit InvalidTermIrrelevance(TokenGenerator& token_generator) : MetamorphicRelation(token_generator) {};

    std::string mutate_input(std::string input) override;

    bool holds(const std::unordered_set<int>& original, const std::unordered_set<int>& modified) override;

    constexpr std::string get_name() const override {
        return "invalid_term_irrelevance";
    };
};
} // namespace matf::verification::metamorphic_testing::relations
