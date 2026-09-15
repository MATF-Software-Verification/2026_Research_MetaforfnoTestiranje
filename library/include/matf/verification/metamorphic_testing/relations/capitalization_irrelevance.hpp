#pragma once
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

namespace matf::verification::metamorphic_testing::relations {
    
class CapitalizationIrrelevance : public MetamorphicRelation {
public:
    std::string mutate_input(std::string& input) override;

    bool holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) override;
};
}