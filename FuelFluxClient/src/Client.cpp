#include "fuelflux/Client.h"
#include "fuelflux/HttpClient.h"
#include "fuelflux/Cache.h"
#include "fuelflux/OfflineQueue.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>

namespace fuelflux {

Client::Client(const Config& cfg)
    : cfg_(cfg),
      http_(std::make_shared<HttpClient>(cfg.baseUrl)),
      cache_(std::make_shared<Cache>(cfg.dbPath)),
      queue_(std::make_shared<OfflineQueue>()) {

    bg_ = std::thread(&Client::backgroundLoop, this);
}

Client::~Client() {
    running_ = false;
    if(bg_.joinable()) bg_.join();
}

bool Client::authorize(const std::string& controllerUid,
                       const std::string& userUid) {
    nlohmann::json req = {
        {"PumpControllerUid", controllerUid},
        {"UserUid", userUid}
    };

    try {
        auto res = http_->post("/api/pump/authorize", req);
        token_ = res.at("Token").get<std::string>();

        // Update fuel tanks cache -----------------------------
        if(res.contains("FuelTanks")) {
            std::vector<FuelTankItem> tanks;
            for(const auto& jt : res["FuelTanks"]) {
                FuelTankItem t;
                t.number = jt.at("number").get<int>();
                t.volume = jt.at("volume").get<double>();
                tanks.push_back(t);
            }
            cache_->updateFuelTanks(tanks);
        }

        // Update user for current session ---------------------
        UserItem self;
        self.uid = userUid;
        self.roleId = res.at("RoleId").get<int>();
        if(res.contains("Allowance") && !res["Allowance"].is_null())
            self.allowance = res["Allowance"].get<double>();
        cache_->updateUsers({self});

        cache_->save();
        return true;
    } catch(const std::exception& ex) {
        std::cerr << "[fuelflux] authorize failed: " << ex.what() << std::endl;
        return false;
    }
}

void Client::deauthorize() {
    try {
        http_->post("/api/pump/deauthorize", nlohmann::json::object(), token_);
    } catch(...) {
        // Ignore errors
    }
    token_.clear();
}

bool Client::reportFuelIntake(int tankNumber, double intakeVolume) {
    nlohmann::json body = {
        {"TankNumber", tankNumber},
        {"IntakeVolume", intakeVolume}
    };
    if(networkOK()) {
        try {
            http_->post("/api/pump/fuelintake", body, token_);
            return true;
        } catch(...) {}
    }

    // Network down or failed – queue it
    queue_->enqueue({"POST", "/api/pump/fuelintake", body, token_});
    return false;
}

bool Client::reportRefuel(int tankNumber, double fuelVolume) {
    nlohmann::json body = {
        {"TankNumber", tankNumber},
        {"FuelVolume", fuelVolume}
    };
    if(networkOK()) {
        try {
            http_->post("/api/pump/refuel", body, token_);
            return true;
        } catch(...) {}
    }
    queue_->enqueue({"POST", "/api/pump/refuel", body, token_});
    return false;
}

void Client::syncUsers(int first, int number) {
    try {
        auto res = http_->get("/api/pump/user?first=" + std::to_string(first) +
                              "&number=" + std::to_string(number), token_);
        if(!res.is_array()) return;

        std::vector<UserItem> list;
        for(const auto& ju : res) {
            UserItem u;
            u.uid = ju.at("Uid").get<std::string>();
            u.roleId = ju.at("RoleId").get<int>();
            if(ju.contains("Allowance") && !ju["Allowance"].is_null())
                u.allowance = ju["Allowance"].get<double>();
            list.push_back(std::move(u));
        }
        cache_->updateUsers(list);
        cache_->save();
    } catch(const std::exception& ex) {
        std::cerr << "[fuelflux] syncUsers failed: " << ex.what() << std::endl;
    }
}

void Client::syncFuelTanks() {
    // Not in API spec; you might implement a GET /api/pump/tanks endpoint
}

void Client::backgroundLoop() {
    using namespace std::chrono_literals;
    while(running_) {
        std::this_thread::sleep_for(cfg_.cacheRefresh);

        if(!networkOK()) continue;

        flushQueue();
        syncUsers();
        // add syncFuelTanks() when available
    }
}

bool Client::networkOK() {
    // Very naive implementation.  Replace with proper connection test.
    try {
        http_->get("/health");
        return true;
    } catch(...) {
        return false;
    }
}

void Client::flushQueue() {
    OfflineQueue::Item it;
    while(queue_->try_pop(it)) {
        try {
            if(it.method == "POST")
                http_->post(it.endpoint, it.body, it.token);
            else
                http_->get(it.endpoint, it.token);
        } catch(...) {
            // Abort flush on first failure and put item back
            queue_->enqueue(std::move(it));
            break;
        }
    }
}

} // namespace fuelflux
