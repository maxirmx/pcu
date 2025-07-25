#include "fuelflux/Cache.h"
#include <stdexcept>

namespace fuelflux {

Cache::Cache(const std::string& dbPath) {
    openDb(dbPath);
    createSchema();
    load();
}

Cache::~Cache() {
    save();
    if(db_) sqlite3_close(db_);
}

void Cache::openDb(const std::string& path) {
    if(sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Failed to open SQLite DB");
    }
}

void Cache::createSchema() {
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS fuel_tanks (number INTEGER PRIMARY KEY, volume REAL);"
        "CREATE TABLE IF NOT EXISTS users (uid TEXT PRIMARY KEY, roleId INTEGER, allowance REAL);";
    char* err = nullptr;
    if(sqlite3_exec(db_, ddl, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("Failed to create schema: " + msg);
    }
}

void Cache::load() {
    std::lock_guard<std::mutex> lock(mtx_);
    fuelTanks_.clear();
    users_.clear();

    const char* q1 = "SELECT number, volume FROM fuel_tanks;";
    sqlite3_stmt* stmt = nullptr;
    if(sqlite3_prepare_v2(db_, q1, -1, &stmt, nullptr) == SQLITE_OK) {
        while(sqlite3_step(stmt) == SQLITE_ROW) {
            FuelTankItem it;
            it.number = sqlite3_column_int(stmt, 0);
            it.volume = sqlite3_column_double(stmt, 1);
            fuelTanks_.push_back(it);
        }
    }
    sqlite3_finalize(stmt);

    const char* q2 = "SELECT uid, roleId, allowance FROM users;";
    if(sqlite3_prepare_v2(db_, q2, -1, &stmt, nullptr) == SQLITE_OK) {
        while(sqlite3_step(stmt) == SQLITE_ROW) {
            UserItem u;
            u.uid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            u.roleId = sqlite3_column_int(stmt, 1);
            if(sqlite3_column_type(stmt, 2) != SQLITE_NULL)
                u.allowance = sqlite3_column_double(stmt, 2);
            users_.emplace(u.uid, std::move(u));
        }
    }
    sqlite3_finalize(stmt);
}

void Cache::save() {
    std::lock_guard<std::mutex> lock(mtx_);
    char* err = nullptr;
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err);

    sqlite3_exec(db_, "DELETE FROM fuel_tanks;", nullptr, nullptr, &err);
    const char* ins1 = "INSERT INTO fuel_tanks(number, volume) VALUES(?,?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, ins1, -1, &stmt, nullptr);
    for(const auto& ft: fuelTanks_) {
        sqlite3_bind_int(stmt, 1, ft.number);
        sqlite3_bind_double(stmt, 2, ft.volume);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_exec(db_, "DELETE FROM users;", nullptr, nullptr, &err);
    const char* ins2 = "INSERT INTO users(uid, roleId, allowance) VALUES(?,?,?);";
    sqlite3_prepare_v2(db_, ins2, -1, &stmt, nullptr);
    for(const auto& kv : users_) {
        sqlite3_bind_text(stmt, 1, kv.second.uid.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, kv.second.roleId);
        if(kv.second.allowance)
            sqlite3_bind_double(stmt, 3, *kv.second.allowance);
        else
            sqlite3_bind_null(stmt, 3);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    sqlite3_exec(db_, "END TRANSACTION;", nullptr, nullptr, &err);
}

void Cache::updateFuelTanks(const std::vector<FuelTankItem>& list) {
    std::lock_guard<std::mutex> lock(mtx_);
    fuelTanks_ = list;
}

std::vector<FuelTankItem> Cache::fuelTanks() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return fuelTanks_;
}

void Cache::updateUsers(const std::vector<UserItem>& list) {
    std::lock_guard<std::mutex> lock(mtx_);
    users_.clear();
    for(const auto& u : list) users_.emplace(u.uid, u);
}

std::optional<UserItem> Cache::user(const std::string& uid) const {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = users_.find(uid);
    if(it == users_.end()) return std::nullopt;
    return it->second;
}

} // namespace fuelflux
