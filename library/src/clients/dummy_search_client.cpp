#include <matf/verification/metamorphic_testing/clients/dummy_search_client.hpp>

#include <spdlog/spdlog.h>

namespace matf::verification::metamorphic_testing::clients {

void DummySearchClient::index_document(int id, std::span<const std::byte> content) {
    spdlog::info("indexed document {} ({} bytes)", id, content.size());
}

std::unordered_set<int> DummySearchClient::query(std::string input) {
    spdlog::info("querying for input {}", input);
    return {1};
}
} // namespace matf::verification::metamorphic_testing::clients
