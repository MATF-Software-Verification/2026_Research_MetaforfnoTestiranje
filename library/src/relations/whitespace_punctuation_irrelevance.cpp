#include <matf/verification/metamorphic_testing/relations/whitespace_punctuation_irrelevance.hpp>

#include <random>
#include <utility>
#include <vector>

namespace matf::verification::metamorphic_testing::relations {

namespace {

const std::vector<std::string> paddings = {"", " ", "  ", "\t", "\n", " \t ", " "};

const std::vector<std::pair<std::string, std::string>> wrappers = {
    {"\"", "\""}, {"'", "'"}, {"(", ")"}, {"[", "]"}, {"{", "}"}, {"<", ">"}, {"“", "”"}, {"«", "»"}, {"*", "*"},
    {"/", "/"},   {"|", "|"}, {"`", "`"}, {"", ","},  {"", "."},  {"", ";"},  {"", ":"},  {"", "!"},  {"", "?"},
    {"", "..."},  {"", "…"},  {"-", ""},  {"–", ""},  {"—", ""},  {"#", ""},  {"@", ""},  {"&", ""},  {"+", ""},
    {"=", ""},    {"%", ""},  {"$", ""},  {"^", ""},  {"~", ""},  {"\\", ""},
};

template <typename T> const T& pick(const std::vector<T>& options, std::mt19937& gen) {
    std::uniform_int_distribution<std::size_t> dist(0, options.size() - 1);
    return options[dist(gen)];
}

} // namespace

std::string WhitespacePunctuationIrrelevance::mutate_input(std::string input) {
    const auto& [open, close] = pick(wrappers, token_generator.engine());
    input = open + input + close;
    return pick(paddings, token_generator.engine()) + input + pick(paddings, token_generator.engine());
}

bool WhitespacePunctuationIrrelevance::holds(const std::unordered_set<int>& original,
                                             const std::unordered_set<int>& modified) {
    return original == modified;
}
} // namespace matf::verification::metamorphic_testing::relations
