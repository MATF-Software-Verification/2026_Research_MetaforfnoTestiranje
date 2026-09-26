#pragma once
#include <cstdint>
#include <random>
#include <string>
#include <vector>
namespace matf::verification::metamorphic_testing {
class TokenGenerator {
public:
    TokenGenerator(std::vector<std::string> tokens, std::uint32_t seed);
    static TokenGenerator from_file(const std::string& file_path, std::uint32_t seed);

    std::string get_random_token();
    std::string get_invalid_token();
    std::mt19937& engine() {
        return gen;
    }

private:
    std::vector<std::string> tokens;
    std::mt19937 gen;
};
} // namespace matf::verification::metamorphic_testing
