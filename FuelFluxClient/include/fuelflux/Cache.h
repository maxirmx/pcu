#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <sqlite3.h>

#include "fuelflux/Models.h"

namespace fuelflux {

/**
 * Thread‑safe in‑memory cache which mirrors the FuelTank and User tables.
 * Data are persisted in an on‑disk SQLite database for fast start‑up.
 */
class Cache {
public:
    explicit Cache(const std::string& dbPath);
    ~Cache();

    // --- Fuel tanks ---------------------------------------------------------
    void updateFuelTanks(const std::vector<FuelTankItem>& list);
    std::vector<FuelTankItem> fuelTanks() const;

    // --- Users --------------------------------------------------------------
    void updateUsers(const std::vector<UserItem>& list);
    std::optional<UserItem> user(const std::string& uid) const;

    // --- Persistence --------------------------------------------------------
    void load();   ///< load cache from SQLite
    void save();   ///< flush memory state to SQLite

private:
    void openDb(const std::string& path);
    void createSchema();

    sqlite3* db_ = nullptr;

    mutable std::mutex mtx_;
    std::vector<FuelTankItem>                       fuelTanks_;
    std::unordered_map<std::string, UserItem>       users_;
};

} // namespace fuelflux
