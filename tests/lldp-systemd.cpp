#include <sdbus-c++/sdbus-c++.h>
#include "doctest_integration.h"
#include "test_log_setup.h"
#include "LLDP.h"

#if 1 // LIST_NEIGHBORS_RUN
TEST_CASE("Connection")
{
    TEST_INIT_LOGS;

    auto dbusConnection = sdbus::createSystemBusConnection();
    auto dataProvider = std::make_shared<lldp::lldp::LLDPDataProvider>("/run/systemd/netif/lldp", *dbusConnection);
    [[maybe_unused]] auto x = dataProvider->getNeighbors();
}
#endif
