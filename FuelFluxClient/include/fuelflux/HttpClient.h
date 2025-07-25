#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace fuelflux {

/**
 * Very thin wrapper around libcurl that returns JSON documents.
 * In production you probably want connection pooling, better error handling,
 * TLS pinning, etc.  For now we only implement the primitives we need.
 */
class HttpClient {
public:
    explicit HttpClient(std::string baseUrl);
    ~HttpClient();

    nlohmann::json post(const std::string& endpoint,
                        const nlohmann::json& body,
                        const std::string& token = {});

    nlohmann::json get(const std::string& endpoint,
                       const std::string& token = {});

private:
    std::string base_;
};

} // namespace fuelflux
