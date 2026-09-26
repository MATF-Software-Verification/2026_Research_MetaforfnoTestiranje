#pragma once
#include <matf/verification/metamorphic_testing/query_operator.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <unordered_set>

namespace matf::verification::metamorphic_testing::clients {

class SearchEngineClient {
public:
    virtual void index_document(int id, std::span<const std::byte> content) = 0;
    virtual std::unordered_set<int> query(std::string input, QueryOperator op) = 0;
    virtual ~SearchEngineClient() = default;
};

} // namespace matf::verification::metamorphic_testing::clients
