#pragma once
#include <matf/verification/metamorphic_testing/clients/search_engine_client.hpp>

namespace matf::verification::metamorphic_testing::clients {


class DummySearchClient : public SearchEngineClient {
public:
    void index_document() override;
    std::unordered_set<int> query(std::string input) override;
};


} // namespace matf::verification::metamorphic_testing::clients