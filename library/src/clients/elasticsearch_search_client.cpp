#include <matf/verification/metamorphic_testing/clients/elasticsearch_search_client.hpp>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <stdexcept>
#include <thread>

namespace matf::verification::metamorphic_testing::clients {

namespace {

constexpr auto pipeline_id = "pdf_pipeline";
constexpr auto json_content_type = "application/json";

void configure(httplib::Client& http) {
    http.set_connection_timeout(2, 0);
    http.set_read_timeout(60, 0);
}

void expect_ok(const httplib::Result& res, const std::string& action) {
    if (!res) {
        throw std::runtime_error("Elasticsearch: " + action + " failed: " + httplib::to_string(res.error()));
    }
    if (res->status < 200 || res->status >= 300) {
        throw std::runtime_error("Elasticsearch: " + action + " failed with HTTP " + std::to_string(res->status) + ": " + res->body);
    }
}

std::string base64_encode(std::span<const std::byte> data) {
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve((data.size() + 2) / 3 * 4);
    for (std::size_t i = 0; i < data.size(); i += 3) {
        const bool has_second = i + 1 < data.size();
        const bool has_third = i + 2 < data.size();
        const unsigned b0 = std::to_integer<unsigned>(data[i]);
        const unsigned b1 = has_second ? std::to_integer<unsigned>(data[i + 1]) : 0u;
        const unsigned b2 = has_third ? std::to_integer<unsigned>(data[i + 2]) : 0u;
        const unsigned triple = (b0 << 16) | (b1 << 8) | b2;

        out.push_back(alphabet[(triple >> 18) & 0x3F]);
        out.push_back(alphabet[(triple >> 12) & 0x3F]);
        out.push_back(has_second ? alphabet[(triple >> 6) & 0x3F] : '=');
        out.push_back(has_third ? alphabet[triple & 0x3F] : '=');
    }
    return out;
}

} // namespace

ElasticsearchSearchClient::ElasticsearchSearchClient(std::string host, int port, std::string index_name)
    : host(std::move(host)), port(port), index_name(std::move(index_name)) {
    wait_until_ready();
    ensure_pipeline();
    ensure_index();
}

void ElasticsearchSearchClient::wait_until_ready() {
    constexpr int max_attempts = 30;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        httplib::Client http(host, port);
        configure(http);

        auto res = http.Get("/_cluster/health?wait_for_status=yellow&timeout=30s");
        if (res && res->status == 200) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    throw std::runtime_error("Elasticsearch at " + host + ":" + std::to_string(port) + " did not become ready");
}

void ElasticsearchSearchClient::ensure_pipeline() {
    const auto pipeline = nlohmann::json::parse(R"({
        "description": "Extract text from PDF/Office/etc attachments",
        "processors": [
            { "attachment": { "field": "data", "target_field": "attachment", "indexed_chars": -1 } },
            { "remove": { "field": "data" } }
        ]
    })");

    httplib::Client http(host, port);
    configure(http);
    expect_ok(http.Put(std::string("/_ingest/pipeline/") + pipeline_id, pipeline.dump(), json_content_type),
              "create ingest pipeline");
}

void ElasticsearchSearchClient::ensure_index() {
    auto index = nlohmann::json::parse(R"({
        "mappings": {
            "properties": {
                "data": { "type": "binary", "doc_values": false }
            }
        }
    })");
    index["settings"]["index.default_pipeline"] = pipeline_id;

    httplib::Client http(host, port);
    configure(http);
    auto res = http.Put("/" + index_name, index.dump(), json_content_type);

    const bool already_exists = res && res->status == 400 &&
                                res->body.find("resource_already_exists_exception") != std::string::npos;
    if (already_exists) {
        return;
    }
    expect_ok(res, "create index '" + index_name + "'");
}

void ElasticsearchSearchClient::index_document(int id, std::span<const std::byte> content) {
    nlohmann::json body;
    body["data"] = base64_encode(content);

    httplib::Client http(host, port);
    configure(http);

    const auto path = "/" + index_name + "/_doc/" + std::to_string(id) + "?refresh=true";
    expect_ok(http.Put(path, body.dump(), json_content_type), "index document " + std::to_string(id));
}

std::unordered_set<int> ElasticsearchSearchClient::query(std::string input) {
    nlohmann::json body;
    body["query"]["match"]["attachment.content"] = input;
    body["_source"] = false;
    body["size"] = 10000;

    httplib::Client http(host, port);
    configure(http);
    auto res = http.Post("/" + index_name + "/_search", body.dump(), json_content_type);
    expect_ok(res, "search");

    const auto response = nlohmann::json::parse(res->body);
    std::unordered_set<int> ids;
    for (const auto& hit : response.at("hits").at("hits")) {
        ids.insert(std::stoi(hit.at("_id").get<std::string>()));
    }
    return ids;
}

} // namespace matf::verification::metamorphic_testing::clients
