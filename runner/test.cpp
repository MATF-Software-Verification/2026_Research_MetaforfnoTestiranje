#include <matf/verification/metamorphic_testing/clients/elasticsearch_search_client.hpp>
#include <matf/verification/metamorphic_testing/pdf/pdf_splitter.hpp>
#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/token_generator.hpp>
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

namespace {
constexpr std::string TOKENS_FILE_PATH = "tokens.txt";
void index_pdf(mt::clients::ElasticsearchSearchClient& client, std::string path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        spdlog::error("cannot open {}", path);
        return;
    }
    const std::vector<char> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const auto pages = mt::pdf::split_pages(std::as_bytes(std::span(raw)));
    for (int i = 0; i < pages.size(); ++i) {
        client.index_document((i + 1), pages[i]);
    }
    spdlog::info("indexed {} pages", pages.size());
}

void write_tokens(mt::clients::ElasticsearchSearchClient& client) {
    const auto tokens = client.get_tokens();

    std::ofstream tokens_file(TOKENS_FILE_PATH);
    if (!tokens_file) {
        throw std::runtime_error("cannot write " + TOKENS_FILE_PATH);
    }
    for (const auto& token : tokens) {
        tokens_file << token << "\n";
    }
    tokens_file.close();
    spdlog::info("wrote {} tokens to {}", tokens.size(), TOKENS_FILE_PATH);
}
} // namespace

int main(int argc, char** argv) {
    // Diagnostics go to stderr so that stdout carries only the results.
    spdlog::set_default_logger(spdlog::stderr_color_mt("test_run"));

    if (argc != 2) {
        fmt::print(stderr, "usage: {} <file.pdf>\n", argv[0]);
        return 1;
    }

    try {
        mt::clients::ElasticsearchSearchClient client;
        index_pdf(client, argv[1]);

        write_tokens(client);

        mt::TokenGenerator token_generator(TOKENS_FILE_PATH);
        mt::relations::CapitalizationIrrelevance relation(token_generator);
        mt::Verifier verifier(client);
        verifier.verify_relation(token_generator.get_random_token(), relation);
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
