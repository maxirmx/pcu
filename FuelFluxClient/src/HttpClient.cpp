#include "fuelflux/HttpClient.h"
#include <curl/curl.h>
#include <stdexcept>
#include <sstream>

namespace fuelflux {

namespace {

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realSize = size * nmemb;
    std::string* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), realSize);
    return realSize;
}

} // anonymous

HttpClient::HttpClient(std::string baseUrl)
    : base_(std::move(baseUrl)) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

nlohmann::json HttpClient::post(const std::string& endpoint,
                                const nlohmann::json& body,
                                const std::string& token) {
    CURL* curl = curl_easy_init();
    if(!curl) throw std::runtime_error("curl_easy_init failed");

    std::string url = base_ + endpoint;
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if(!token.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    }

    std::string bodyStr = body.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) {
        throw std::runtime_error("HTTP POST failed: " + std::string(curl_easy_strerror(res)));
    }

    return nlohmann::json::parse(response);
}

nlohmann::json HttpClient::get(const std::string& endpoint,
                               const std::string& token) {
    CURL* curl = curl_easy_init();
    if(!curl) throw std::runtime_error("curl_easy_init failed");

    std::string url = base_ + endpoint;
    std::string response;
    struct curl_slist* headers = nullptr;
    if(!token.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) {
        throw std::runtime_error("HTTP GET failed: " + std::string(curl_easy_strerror(res)));
    }

    return nlohmann::json::parse(response.empty() ? "{}" : response);
}

} // namespace fuelflux
