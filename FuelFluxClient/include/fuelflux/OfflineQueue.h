#pragma once
#include <mutex>
#include <queue>
#include <string>
#include <nlohmann/json.hpp>

namespace fuelflux {

/**
 * Stores requests that could not be sent because the network is down.
 * At the moment this is an in‑memory FIFO; you may want to persist it
 * to SQLite in the future for crash‑safety.
 */
class OfflineQueue {
public:
    struct Item {
        std::string      method;   ///< "POST" | "GET"
        std::string      endpoint;
        nlohmann::json   body;
        std::string      token;
    };

    void enqueue(Item&&);
    bool empty() const;

    /// Pop the earliest item; returns false if queue empty
    bool try_pop(Item& out);

private:
    mutable std::mutex mtx_;
    std::queue<Item>   q_;
};

} // namespace fuelflux
