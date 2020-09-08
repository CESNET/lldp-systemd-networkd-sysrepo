#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>
#include "LLDP.h"
#include "configure.cmake.h"
#include "dbus-helpers/dbus_server.h"
#include "doctest_integration.h"
#include "test_log_setup.h"

using namespace std::literals;

namespace lldp::lldp {
bool operator==(const NeighborEntry& a, const NeighborEntry& b)
{
    return std::tie(a.m_portId, a.m_properties) == std::tie(b.m_portId, b.m_properties);
}
} /* namespace lldp::lldp */

TEST_CASE("Parsing with the mock")
{
    TEST_INIT_LOGS;

    const auto serverBus = "local.pid" + std::to_string(getpid()) + ".org.freedesktop.network1";
    auto dbusServerConnection = sdbus::createSessionBusConnection(serverBus);
    dbusServerConnection->enterEventLoopAsync();

    std::vector<std::pair<int, std::string>> links;
    std::string dataDir;
    std::vector<lldp::lldp::NeighborEntry> expected;

    SECTION("LLDP active on a single link")
    {
        links = {{1, "lo"}, {2, "enp0s25"}, {3, "wlp3s0"}, {4, "tun0"}, {5, "br-53662f640039"}, {6, "docker0"}, {7, "br-e78120c0adda"}, {8, "ve-image"}};
        dataDir = "single-link";
        expected = {{"ve-image", {{"remoteSysName", "image"}, {"remotePortId", "host0"}, {"remoteChassisId", "7062a9e41c924ac6942da39c56e6b820"}}}};
    }

    SECTION("No LLDP enabled")
    {
        links = {{1, "lo"}, {2, "enp0s25"}, {3, "wlp3s0"}, {4, "tun0"}, {5, "br-53662f640039"}, {6, "docker0"}, {7, "br-e78120c0adda"}, {8, "ve-image"}};
        dataDir = "no-link";
        expected = {};
    }

    auto dbusServer = DbusServer(*dbusServerConnection);
    dbusServer.setLinks(links); // intentionally not mocking DbusMockServer::ListLinks but using explicit set/get pattern so I can avoid an unneccesary dependency on trompeloeil

    auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>(std::filesystem::path(CMAKE_CURRENT_SOURCE_DIR "/tests/files/"s) / dataDir, sdbus::createSessionBusConnection(), serverBus);

    REQUIRE(lldp->getNeighbors() == expected);
}

#if LIST_NEIGHBORS_RUN
TEST_CASE("Real systemd")
{
    TEST_INIT_LOGS;

    auto dbusConnection = sdbus::createSystemBusConnection();
    auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>("/run/systemd/netif/lldp", *dbusConnection, "org.freedesktop.network1");
    [[maybe_unused]] auto x = lldp->getNeighbors();
}
#endif
