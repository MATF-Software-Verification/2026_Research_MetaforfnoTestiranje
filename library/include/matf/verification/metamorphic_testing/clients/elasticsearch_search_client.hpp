#pragma once
#include <matf/verification/metamorphic_testing/clients/search_engine_client.hpp>

#include <memory>
#include <string>
#include <vector>

namespace httplib {
class Client;
}

namespace matf::verification::metamorphic_testing::clients {


class ElasticsearchSearchClient : public SearchEngineClient {
public:
    explicit ElasticsearchSearchClient(std::string host = "localhost", int port = 9200, std::string index_name = "docs");
    ~ElasticsearchSearchClient() override;

    void index_document(int id, std::span<const std::byte> content) override;
    std::unordered_set<int> query(std::string input) override;
    std::vector<std::string> get_tokens();

private:
    void wait_until_ready();
    void ensure_pipeline();
    void ensure_index();

    std::unique_ptr<httplib::Client> http;
    std::string index_name;
};


} // namespace matf::verification::metamorphic_testing::clients
