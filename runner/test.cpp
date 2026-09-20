#include <matf/verification/metamorphic_testing/clients/elasticsearch_search_client.hpp>
#include <matf/verification/metamorphic_testing/pdf/pdf_splitter.hpp>
#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/verifier.hpp>

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <fstream>
#include <iterator>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace mt = matf::verification::metamorphic_testing;

int main(int argc, char** argv) {
    // Diagnostics go to stderr so that stdout carries only the results.
    spdlog::set_default_logger(spdlog::stderr_color_mt("test_run"));

    if (argc != 3) {
        fmt::print(stderr, "usage: {} <file.pdf> <query>\n", argv[0]);
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        spdlog::error("cannot open {}", argv[1]);
        return 1;
    }
    const std::vector<char> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        const auto pages = mt::pdf::split_pages(std::as_bytes(std::span(raw)));

        mt::clients::ElasticsearchSearchClient client;
        for (int i = 0; i < pages.size(); ++i) {
            client.index_document((i + 1), pages[i]);
        }
        spdlog::info("indexed {} pages", pages.size());

        const auto tokens = client.get_tokens();

        const std::string tokens_path = "tokens.txt";
        std::ofstream tokens_file(tokens_path);
        if (!tokens_file) {
            throw std::runtime_error("cannot write " + tokens_path);
        }
        for (const auto& token : tokens) {
            tokens_file << token << "\n";
        }
        spdlog::info("wrote {} tokens to {}", tokens.size(), tokens_path);

        const auto matches = client.query(argv[2]);
        const std::set<int> sorted_matches(matches.begin(), matches.end());
        fmt::print("pages matching '{}': {}\n", argv[2], fmt::join(sorted_matches, " "));

        mt::relations::CapitalizationIrrelevance relation;
        mt::Verifier verifier(client);
        verifier.verify_relation(argv[2], relation);
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
