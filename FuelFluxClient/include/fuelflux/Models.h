#pragma once
#include <string>
#include <vector>
#include <optional>

namespace fuelflux {

struct FuelTankItem {
    int number = 0;
    double volume = 0.0;
};

struct UserItem {
    std::string uid;
    int         roleId = 0;
    std::optional<double> allowance;
};

} // namespace fuelflux
