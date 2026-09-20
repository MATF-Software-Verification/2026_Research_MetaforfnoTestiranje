#include <matf/verification/metamorphic_testing/clients/elasticsearch_search_client.hpp>
#include <matf/verification/metamorphic_testing/pdf/pdf_splitter.hpp>
#include <matf/verification/metamorphic_testing/relations/capitalization_irrelevance.hpp>
#include <matf/verification/metamorphic_testing/verifier.hpp>

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace mt = matf::verification::metamorphic_testing;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <file.pdf> <query>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "cannot open " << argv[1] << "\n";
        return 1;
    }
    const std::vector<char> raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        const auto pages = mt::pdf::split_pages(std::as_bytes(std::span(raw)));

        mt::clients::ElasticsearchSearchClient client;
        for (int i = 0; i < pages.size(); ++i) {
            client.index_document((i + 1), pages[i]);
        } 
        std::cout << "indexed " << pages.size() << " pages\n";

        const auto tokens = client.get_tokens();

        const std::string tokens_path = "tokens.txt";
        std::ofstream tokens_file(tokens_path);
        if (!tokens_file) {
            throw std::runtime_error("cannot write " + tokens_path);
        }
        for (const auto& token : tokens) {
            tokens_file << token << "\n";
        }
        std::cout << "wrote " << tokens.size() << " tokens to " << tokens_path << "\n";

        const auto matches = client.query(argv[2]);
        std::cout << "pages matching '" << argv[2] << "':";
        for (int page : std::set<int>(matches.begin(), matches.end())) {
            std::cout << " " << page;
        }
        std::cout << std::endl;

        mt::relations::CapitalizationIrrelevance relation;
        mt::Verifier verifier(client);
        verifier.verify_relation(argv[2], relation);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
