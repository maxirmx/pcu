#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <string>

#include "fuelflux/Models.h"

namespace fuelflux {

class HttpClient;
class Cache;
class OfflineQueue;

/**
 * Facade that exposes high‑level API calls and coordinates
 * caching, background sync, offline queue, etc.
 */
class Client {
public:
    struct Config {
        std::string      baseUrl              = "https://fuelflux.sw.consulting:8087";
        std::string      dbPath               = "fuelflux_cache.db";
        std::chrono::seconds cacheRefresh     = std::chrono::seconds{60};
    };

    explicit Client(const Config& cfg);
    ~Client();

    bool authorize(const std::string& controllerUid,
                   const std::string& userUid);

    void deauthorize();

    bool reportFuelIntake(int tankNumber, double intakeVolume);
    bool reportRefuel(int tankNumber, double fuelVolume);

    // Manual sync helpers ----------------------------------------------------
    void syncUsers(int first = 0, int number = 100);
    void syncFuelTanks(); ///< Not in initial API doc, but handy

private:
    void backgroundLoop();
    bool networkOK();
    void flushQueue();

    Config                         cfg_;
    std::shared_ptr<HttpClient>    http_;
    std::shared_ptr<Cache>         cache_;
    std::shared_ptr<OfflineQueue>  queue_;

    std::thread        bg_;
    std::atomic<bool>  running_{true};
    std::string        token_;
};

} // namespace fuelflux
