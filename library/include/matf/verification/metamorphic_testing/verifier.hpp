#pragma once

#include <matf/verification/metamorphic_testing/clients/search_engine_client.hpp>
#include <matf/verification/metamorphic_testing/query_operator.hpp>
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

#include <spdlog/spdlog.h>

namespace matf::verification::metamorphic_testing {

class Verifier {
public:
    Verifier(clients::SearchEngineClient& client) : client(client) {};
    void verify_relation(relations::MetamorphicRelation& relation) {
        const auto input = relation.generate_input();
        const auto modified_input = relation.mutate_input(input);
        const auto op = relation.get_operator();
        spdlog::info("Checking if {} holds for original: \"{}\" and modified: \"{}\" (operator {})",
                     relation.get_name(), input, modified_input, to_string(op));

        auto new_result = client.query(modified_input, op);
        auto original_result = client.query(input, op);

        auto relation_holds = relation.holds(original_result, new_result);
        spdlog::info("result: {}", relation_holds);
    }

private:
    clients::SearchEngineClient& client;
};
} // namespace matf::verification::metamorphic_testing
