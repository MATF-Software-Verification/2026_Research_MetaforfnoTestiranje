#include <matf/verification/metamorphic_testing/clients/elasticsearch_search_client.hpp>
#include <matf/verification/metamorphic_testing/pdf/pdf_splitter.hpp>
#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/duplicate_term_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/input_permutation.hpp>
#include <matf/verification/metamorphic_testing/relations/invalid_term_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/relations/invalid_term_relevance.hpp>
#include <matf/verification/metamorphic_testing/relations/multiple_term_reduction.hpp>
#include <matf/verification/metamorphic_testing/relations/term_addition_monotonicity.hpp>
#include <matf/verification/metamorphic_testing/relations/whitespace_punctuation_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/token_generator.hpp>
#include <matf/verification/metamorphic_testing/verifier.hpp>

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace mt = matf::verification::metamorphic_testing;

namespace {
constexpr std::string TOKENS_FILE_PATH = "tokens.txt";

void index_pdf(mt::clients::ElasticsearchSearchClient& client, const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    const std::vector<char> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const auto pages = mt::pdf::split_pages(std::as_bytes(std::span(raw)));
    for (std::size_t i = 0; i < pages.size(); ++i) {
        client.index_document(static_cast<int>(i + 1), pages[i]);
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

int run(const std::string& pdf_path, std::uint32_t seed) {
    spdlog::info("seed: {}", seed);

    mt::clients::ElasticsearchSearchClient client;
    index_pdf(client, pdf_path);
    write_tokens(client);

    auto token_generator = mt::TokenGenerator::from_file(TOKENS_FILE_PATH, seed);
    mt::Verifier verifier(client);

    std::unique_ptr<mt::relations::MetamorphicRelation> relations[] = {
        std::make_unique<mt::relations::CapitalizationIrrelevance>(token_generator),
        std::make_unique<mt::relations::WhitespacePunctuationIrrelevance>(token_generator),
        std::make_unique<mt::relations::TermAdditionMonotonicity>(token_generator),
        std::make_unique<mt::relations::DuplicateTermIrrelevance>(token_generator),
        std::make_unique<mt::relations::MultipleTermReduction>(token_generator),
        std::make_unique<mt::relations::InputPermutation>(token_generator),
        std::make_unique<mt::relations::InvalidTermIrrelevance>(token_generator),
        std::make_unique<mt::relations::InvalidTermRelevance>(token_generator),
    };

    std::vector<std::string> failed;
    for (const auto& relation : relations) {
        if (!verifier.verify_relation(*relation)) {
            failed.push_back(relation->get_name());
        }
    }

    if (failed.empty()) {
        fmt::print("SUCCESS: all {} relations hold (seed {})\n", std::size(relations), seed);
        return 0;
    }
    fmt::print("FAILURE: {} of {} relations failed (seed {}): {}\n", failed.size(), std::size(relations), seed,
               fmt::join(failed, ", "));
    return 1;
}
} // namespace

int main(int argc, char** argv) {
    spdlog::set_default_logger(spdlog::stderr_color_mt("test_run"));

    if (argc != 2 && !(argc == 4 && std::string(argv[2]) == "--seed")) {
        fmt::print(stderr, "usage: {} <file.pdf> [--seed <n>]\n", argv[0]);
        return 1;
    }

    try {
        const std::uint32_t seed = argc == 4 ? std::stoul(argv[3]) : std::random_device{}();
        return run(argv[1], seed);
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
