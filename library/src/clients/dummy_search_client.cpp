#include <iostream>
#include <matf/verification/metamorphic_testing/clients/dummy_search_client.hpp>

namespace matf::verification::metamorphic_testing::clients {

void DummySearchClient::index_document(int id, std::span<const std::byte> content) {
    std::cout << "indexed document " << id << " (" << content.size() << " bytes)" << std::endl;
}

std::unordered_set<int> DummySearchClient::query(std::string input) {
    std::cout << "querying for input " << input << std::endl;
    return {1};
}
} // namespace matf::verification::metamorphic_testing::clients
