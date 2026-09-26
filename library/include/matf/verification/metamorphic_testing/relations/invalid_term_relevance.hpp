#pragma once
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

namespace matf::verification::metamorphic_testing::relations {

class InvalidTermRelevance : public MetamorphicRelation {
public:
    explicit InvalidTermRelevance(TokenGenerator& token_generator) : MetamorphicRelation(token_generator) {};

    std::string mutate_input(std::string input) override;

    bool holds(const std::unordered_set<int>& original, const std::unordered_set<int>& modified) override;

    constexpr std::string get_name() const override {
        return "invalid_term_relevance";
    };

    constexpr QueryOperator get_operator() const override {
        return QueryOperator::And;
    }
};
} // namespace matf::verification::metamorphic_testing::relations
