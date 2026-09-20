#pragma once
#include <string>
#include <vector>
namespace matf::verification::metamorphic_testing {
class TokenGenerator {
public:
    explicit TokenGenerator(std::string file_path);
    std::string get_random_token();

private:
    std::string token_file_path;
    std::vector<std::string> tokens;
};
} // namespace matf::verification::metamorphic_testing
