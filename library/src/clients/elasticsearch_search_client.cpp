#include <matf/verification/metamorphic_testing/clients/elasticsearch_search_client.hpp>

#include <b64/cencode.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <set>
#include <stdexcept>
#include <thread>

namespace matf::verification::metamorphic_testing::clients {

namespace {

constexpr auto pipeline_id = "pdf_pipeline";
constexpr auto json_content_type = "application/json";

void configure(httplib::Client& http) {
    http.set_connection_timeout(2, 0);
    http.set_read_timeout(60, 0);
    http.set_keep_alive(true);
}

void expect_ok(const httplib::Result& res, const std::string& action) {
    if (!res) {
        throw std::runtime_error("Elasticsearch: " + action + " failed: " + httplib::to_string(res.error()));
    }
    if (res->status < 200 || res->status >= 300) {
        throw std::runtime_error("Elasticsearch: " + action + " failed with HTTP " + std::to_string(res->status) +
                                 ": " + res->body);
    }
}

std::string base64_encode(std::span<const std::byte> data) {
    base64_encodestate state;
    base64_init_encodestate(&state);

    std::string out(base64_encode_length(data.size(), &state), '\0');
    std::size_t written = base64_encode_block(data.data(), data.size(), out.data(), &state);
    written += base64_encode_blockend(out.data() + written, &state);
    out.resize(written);
    return out;
}

} // namespace

ElasticsearchSearchClient::ElasticsearchSearchClient(std::string host, int port, std::string index_name)
    : http(std::make_unique<httplib::Client>(host, port)), index_name(std::move(index_name)) {
    configure(*http);
    wait_until_ready();
    ensure_pipeline();
    recreate_index();
}

ElasticsearchSearchClient::~ElasticsearchSearchClient() = default;

void ElasticsearchSearchClient::wait_until_ready() {
    constexpr int max_attempts = 30;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        auto res = http->Get("/_cluster/health?wait_for_status=yellow&timeout=30s");
        if (res && res->status == 200) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    throw std::runtime_error("Elasticsearch at " + http->host() + ":" + std::to_string(http->port()) +
                             " did not become ready");
}

void ElasticsearchSearchClient::ensure_pipeline() {
    const auto pipeline = nlohmann::json::parse(R"({
        "description": "Extract text from PDF/Office/etc attachments",
        "processors": [
            { "attachment": { "field": "data", "target_field": "attachment", "indexed_chars": -1 } },
            { "remove": { "field": "data" } }
        ]
    })");

    expect_ok(http->Put(std::string("/_ingest/pipeline/") + pipeline_id, pipeline.dump(), json_content_type),
              "create ingest pipeline");
}

void ElasticsearchSearchClient::recreate_index() {
    auto deleted = http->Delete("/" + index_name);
    if (!deleted || deleted->status != 404) {
        expect_ok(deleted, "delete index '" + index_name + "'");
    }

    auto index = nlohmann::json::parse(R"({
        "mappings": {
            "properties": {
                "data": { "type": "binary", "doc_values": false }
            }
        }
    })");
    index["settings"]["index.default_pipeline"] = pipeline_id;

    expect_ok(http->Put("/" + index_name, index.dump(), json_content_type), "create index '" + index_name + "'");
}

void ElasticsearchSearchClient::index_document(int id, std::span<const std::byte> content) {
    nlohmann::json body;
    body["data"] = base64_encode(content);

    const auto path = "/" + index_name + "/_doc/" + std::to_string(id) + "?refresh=true";
    expect_ok(http->Put(path, body.dump(), json_content_type), "index document " + std::to_string(id));
}

std::unordered_set<int> ElasticsearchSearchClient::query(std::string input) {
    nlohmann::json body;
    body["query"]["match"]["attachment.content"] = input;
    body["_source"] = false;
    body["size"] = 10000;

    auto res = http->Post("/" + index_name + "/_search", body.dump(), json_content_type);
    expect_ok(res, "search");

    const auto response = nlohmann::json::parse(res->body);
    std::unordered_set<int> ids;
    for (const auto& hit : response.at("hits").at("hits")) {
        ids.insert(std::stoi(hit.at("_id").get<std::string>()));
    }
    return ids;
}

std::vector<std::string> ElasticsearchSearchClient::get_tokens() {
    nlohmann::json list_all;
    list_all["query"]["match_all"] = nlohmann::json::object();
    list_all["_source"] = false;
    list_all["size"] = 10000;
    auto listed = http->Post("/" + index_name + "/_search", list_all.dump(), json_content_type);
    expect_ok(listed, "list documents");

    const auto listing = nlohmann::json::parse(listed->body);
    auto ids = nlohmann::json::array();
    for (const auto& hit : listing.at("hits").at("hits")) {
        ids.push_back(hit.at("_id"));
    }
    if (ids.empty()) {
        return {};
    }

    nlohmann::json request;
    request["ids"] = ids;
    request["parameters"]["fields"] = nlohmann::json::array({"attachment.content"});
    request["parameters"]["positions"] = false;
    request["parameters"]["offsets"] = false;
    request["parameters"]["payloads"] = false;
    request["parameters"]["field_statistics"] = false;
    auto vectors = http->Post("/" + index_name + "/_mtermvectors", request.dump(), json_content_type);
    expect_ok(vectors, "read term vectors");

    const auto response = nlohmann::json::parse(vectors->body);
    std::set<std::string> tokens;
    for (auto& doc : response.at("docs")) {
        if (!doc.value("found", false) || !doc.contains("term_vectors") ||
            !doc.at("term_vectors").contains("attachment.content")) {
            continue;
        }
        for (const auto& term : doc.at("term_vectors").at("attachment.content").at("terms").items()) {
            tokens.insert(term.key());
        }
    }
    return {tokens.begin(), tokens.end()};
}

} // namespace matf::verification::metamorphic_testing::clients
