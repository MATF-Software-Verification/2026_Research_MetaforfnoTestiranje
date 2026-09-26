#pragma once
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

namespace matf::verification::metamorphic_testing::relations {

class WhitespacePunctuationIrrelevance : public MetamorphicRelation {
public:
    explicit WhitespacePunctuationIrrelevance(TokenGenerator& token_generator) : MetamorphicRelation(token_generator) {}

    std::string mutate_input(std::string input) override;

    bool holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) override;

    constexpr std::string get_name() const override {
        return "whitespace_punctuation_irrelevance";
    }
};
} // namespace matf::verification::metamorphic_testing::relations
