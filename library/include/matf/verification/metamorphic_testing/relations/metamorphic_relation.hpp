#pragma once
#include <string>
#include <unordered_set>

namespace matf::verification::metamorphic_testing::relations {

class MetamorphicRelation {
public:
    virtual std::string mutate_input(std::string& input) = 0;
    virtual bool holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) = 0;
    virtual ~MetamorphicRelation() = default;
};

} // namespace matf::verification::metamorphic_testing::relations
