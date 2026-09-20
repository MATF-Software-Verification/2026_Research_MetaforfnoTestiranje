#pragma once

#include <matf/verification/metamorphic_testing/token_generator.hpp>

#include <string>
#include <unordered_set>

namespace matf::verification::metamorphic_testing::relations {

class MetamorphicRelation {
public:
    virtual std::string mutate_input(std::string input) = 0;
    virtual bool holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) = 0;
    virtual ~MetamorphicRelation() = default;
    constexpr virtual std::string get_name() const = 0;

protected:
    explicit MetamorphicRelation(TokenGenerator& token_generator) : token_generator(token_generator) {};

    TokenGenerator& token_generator;
};

} // namespace matf::verification::metamorphic_testing::relations
