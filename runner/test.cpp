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

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace mt = matf::verification::metamorphic_testing;

namespace {
constexpr std::string TOKENS_FILE_PATH = "tokens.txt";

struct Options {
    std::string pdf_path;
    std::optional<std::uint32_t> seed;
};

void print_usage(std::string_view program) {
    fmt::print(stderr,
               "usage: {} [--seed <n>] <file.pdf>\n"
               "\n"
               "  --seed <n>  seed for input generation (0 to 4294967295); random if omitted\n",
               program);
}

std::uint32_t parse_seed(std::string_view text) {
    std::uint32_t seed{};
    const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), seed);
    if (ec != std::errc{} || end != text.data() + text.size()) {
        throw std::invalid_argument(fmt::format("invalid seed '{}': expected an integer from 0 to 4294967295", text));
    }
    return seed;
}

Options parse_args(int argc, char** argv) {
    Options options;
    std::optional<std::string> pdf_path;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--seed") {
            if (++i == argc) {
                throw std::invalid_argument("--seed requires a value");
            }
            options.seed = parse_seed(argv[i]);
        } else if (arg.starts_with("-")) {
            throw std::invalid_argument(fmt::format("unknown option '{}'", arg));
        } else if (pdf_path) {
            throw std::invalid_argument(fmt::format("unexpected argument '{}'", arg));
        } else {
            pdf_path = arg;
        }
    }
    if (!pdf_path) {
        throw std::invalid_argument("missing <file.pdf>");
    }
    options.pdf_path = *pdf_path;
    return options;
}

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

std::vector<std::unique_ptr<mt::relations::MetamorphicRelation>> make_relations(mt::TokenGenerator& token_generator) {
    std::vector<std::unique_ptr<mt::relations::MetamorphicRelation>> relations;
    relations.push_back(std::make_unique<mt::relations::CapitalizationIrrelevance>(token_generator));
    relations.push_back(std::make_unique<mt::relations::WhitespacePunctuationIrrelevance>(token_generator));
    relations.push_back(std::make_unique<mt::relations::TermAdditionMonotonicity>(token_generator));
    relations.push_back(std::make_unique<mt::relations::DuplicateTermIrrelevance>(token_generator));
    relations.push_back(std::make_unique<mt::relations::MultipleTermReduction>(token_generator));
    relations.push_back(std::make_unique<mt::relations::InputPermutation>(token_generator));
    relations.push_back(std::make_unique<mt::relations::InvalidTermIrrelevance>(token_generator));
    relations.push_back(std::make_unique<mt::relations::InvalidTermRelevance>(token_generator));
    return relations;
}

// Returns the names of the relations that did not hold.
std::vector<std::string> verify_all(mt::Verifier& verifier,
                                    const std::vector<std::unique_ptr<mt::relations::MetamorphicRelation>>& relations) {
    std::vector<std::string> failed;
    for (const auto& relation : relations) {
        if (!verifier.verify_relation(*relation)) {
            failed.push_back(relation->get_name());
        }
    }
    return failed;
}

int report(const std::vector<std::string>& failed, std::size_t total, std::uint32_t seed) {
    if (failed.empty()) {
        fmt::print("SUCCESS: all {} relations hold (seed {})\n", total, seed);
        return 0;
    }
    fmt::print("FAILURE: {} of {} relations failed (seed {}): {}\n", failed.size(), total, seed,
               fmt::join(failed, ", "));
    return 1;
}

int run(const Options& options) {
    const std::uint32_t seed = options.seed.value_or(std::random_device{}());
    spdlog::info("seed: {}", seed);

    mt::clients::ElasticsearchSearchClient client;
    index_pdf(client, options.pdf_path);
    write_tokens(client);

    auto token_generator = mt::TokenGenerator::from_file(TOKENS_FILE_PATH, seed);
    mt::Verifier verifier(client);

    const auto relations = make_relations(token_generator);
    const auto failed = verify_all(verifier, relations);
    return report(failed, relations.size(), seed);
}
} // namespace

int main(int argc, char** argv) {
    spdlog::set_default_logger(spdlog::stderr_color_mt("test_run"));

    Options options;
    try {
        options = parse_args(argc, argv);
    } catch (const std::invalid_argument& e) {
        fmt::print(stderr, "error: {}\n\n", e.what());
        print_usage(argv[0]);
        return 1;
    }

    try {
        return run(options);
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
