#pragma once
#include <cstddef>
#include <span>
#include <unordered_set>
#include <string>

namespace matf::verification::metamorphic_testing::clients {


class SearchEngineClient {
public:
    virtual void index_document(int id, std::span<const std::byte> content) = 0;
    virtual std::unordered_set<int> query(std::string input) = 0;
    virtual ~SearchEngineClient() = default;
};


} // namespace matf::verification::metamorphic_testing::clients
