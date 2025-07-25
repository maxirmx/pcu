cmake_minimum_required(VERSION 3.15)
project(FuelFluxClient VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch nlohmann/json single-header library
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

add_library(fuelflux STATIC
    src/Client.cpp
    src/Cache.cpp
    src/HttpClient.cpp
    src/OfflineQueue.cpp
)

target_include_directories(fuelflux
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(fuelflux
    PRIVATE
        nlohmann_json::nlohmann_json
        SQLite::SQLite3
        CURL::libcurl
        Threads::Threads
)

option(BUILD_EXAMPLES "Build example executable" ON)
if(BUILD_EXAMPLES)
    add_executable(example samples/example.cpp)
    target_link_libraries(example fuelflux)
endif()
