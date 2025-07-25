#include "fuelflux/Client.h"
#include <iostream>

int main() {
    fuelflux::Client::Config cfg;
    cfg.baseUrl = "https://fuelflux.sw.consulting:8087";
    fuelflux::Client client(cfg);

    if(!client.authorize("1111-1111", "2222-2222-2222-2222")) {
        std::cerr << "Authorization failed\n";
        return 1;
    }

    client.reportRefuel(1, 10.5);

    std::cout << "Queued/ sent refuel request.  Sleeping for 2s...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    client.deauthorize();
    return 0;
}
