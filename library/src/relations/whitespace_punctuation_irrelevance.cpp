#include <matf/verification/metamorphic_testing/relations/whitespace_punctuation_irrelevance.hpp>

#include <random>
#include <utility>
#include <vector>

namespace matf::verification::metamorphic_testing::relations {

namespace {

const std::vector<std::string> paddings = {"", " ", "  ", "\t", "\n", " \t ", " "};

const std::vector<std::pair<std::string, std::string>> wrappers = {
    {"\"", "\""}, {"'", "'"},   {"(", ")"},      {"[", "]"},      {"{", "}"},      {"<", ">"},    {"“", "”"},
    {"«", "»"}, {"*", "*"}, {"/", "/"}, {"|", "|"},     {"`", "`"},      {"", ","},     {"", "."},
    {"", ";"},    {"", ":"},   {"", "!"},        {"", "?"},       {"", "..."},     {"", "…"}, {"-", ""},
    {"–", ""}, {"—", ""}, {"#", ""},   {"@", ""},       {"&", ""},       {"+", ""},     {"=", ""},
    {"%", ""},    {"$", ""},   {"^", ""},        {"~", ""},       {"\\", ""},
};

std::mt19937& rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

template <typename T>
const T& pick(const std::vector<T>& options) {
    static std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> dist(0, options.size() - 1);
    return options[dist(gen)];
}

} // namespace

std::string WhitespacePunctuationIrrelevance::mutate_input(std::string input) {
    const auto& [open, close] = pick(wrappers);
    input = open + input + close;
    return pick(paddings) + input + pick(paddings);
}

bool WhitespacePunctuationIrrelevance::holds(std::unordered_set<int>& original, std::unordered_set<int>& modified) {
    return original == modified;
}
} // namespace matf::verification::metamorphic_testing::relations
