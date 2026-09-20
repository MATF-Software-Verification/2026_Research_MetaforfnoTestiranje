#include <matf/verification/metamorphic_testing/token_generator.hpp>

#include <fstream>
#include <random>
#include <stdexcept>
#include <utility>

namespace {

std::vector<std::string> read_tokens(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("cannot open token file '" + path + "'");
    }

    std::vector<std::string> tokens;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            tokens.push_back(line);
        }
    }
    if (tokens.empty()) {
        throw std::runtime_error("token file '" + path + "' contains no tokens");
    }
    return tokens;
}

} // namespace

matf::verification::metamorphic_testing::TokenGenerator::TokenGenerator(std::string file_path)
    : token_file_path(std::move(file_path)), tokens(read_tokens(token_file_path)) {}

std::string matf::verification::metamorphic_testing::TokenGenerator::get_random_token() {
    static std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> dist(0, tokens.size() - 1);
    return tokens[dist(gen)];
}
