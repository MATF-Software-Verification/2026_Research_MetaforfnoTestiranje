#pragma once
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

namespace matf::verification::metamorphic_testing::relations {

class CapitalizationIrrelevance : public MetamorphicRelation {
public:
    explicit CapitalizationIrrelevance(TokenGenerator& token_generator) : MetamorphicRelation(token_generator) {};

    std::string mutate_input(std::string& input) override;

    bool holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) override;
};
} // namespace matf::verification::metamorphic_testing::relations
