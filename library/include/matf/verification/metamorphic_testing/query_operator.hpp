#pragma once
#include <string_view>

namespace matf::verification::metamorphic_testing {

enum class QueryOperator { Or, And };

constexpr std::string_view to_string(QueryOperator op) {
    return op == QueryOperator::And ? "and" : "or";
}

} // namespace matf::verification::metamorphic_testing
