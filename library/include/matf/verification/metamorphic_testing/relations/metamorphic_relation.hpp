#pragma once

#include <matf/verification/metamorphic_testing/query_operator.hpp>
#include <matf/verification/metamorphic_testing/token_generator.hpp>

#include <string>
#include <unordered_set>

namespace matf::verification::metamorphic_testing::relations {

class MetamorphicRelation {
public:
    virtual std::string generate_input() {
        return token_generator.get_random_token();
    };
    virtual std::string mutate_input(std::string input) = 0;
    virtual bool holds(const std::unordered_set<int>& original, const std::unordered_set<int>& modified) = 0;
    virtual ~MetamorphicRelation() = default;
    constexpr virtual std::string get_name() const = 0;
    constexpr virtual QueryOperator get_operator() const {
        return QueryOperator::Or;
    };

protected:
    explicit MetamorphicRelation(TokenGenerator& token_generator) : token_generator(token_generator) {};

    TokenGenerator& token_generator;
};

} // namespace matf::verification::metamorphic_testing::relations
