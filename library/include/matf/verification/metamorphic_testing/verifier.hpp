#pragma once

#include <matf/verification/metamorphic_testing/clients/search_engine_client.hpp>
#include <matf/verification/metamorphic_testing/relations/metamorphic_relation.hpp>

#include <iostream>
#include <string>

namespace matf::verification::metamorphic_testing {

class Verifier {
public:
    Verifier(clients::SearchEngineClient& client) : client(client) {};
    void verify_relation(std::string input, relations::MetamorphicRelation& relation) {
        auto modified_input = relation.mutate_input(input);

        auto new_result = client.query(modified_input);
        auto original_result = client.query(input);

        auto relation_holds = relation.holds(original_result, new_result);
        std::cout << "result: " << relation_holds << std::endl;
    }

private:
    clients::SearchEngineClient& client;
};
} // namespace matf::verification::metamorphic_testing
