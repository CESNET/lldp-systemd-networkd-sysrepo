#include <sysrepo-cpp/Connection.hpp>
#include <unistd.h>
#include "LLDP.h"
#include "callback.h"
#include "configure.cmake.h"
#include "dbus-helpers/dbus_server.h"
#include "doctest_integration.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

/* Tests are correctly announced by sysrepo */
TEST_CASE("Sysrepo opsdata callback")
{
    TEST_INIT_LOGS;
    TEST_INIT_SYSREPO;

    const auto serverBus = "local.pid" + std::to_string(getpid()) + ".org.freedesktop.network1";
    auto dbusServerConnection = sdbus::createSessionBusConnection(serverBus);
    dbusServerConnection->enterEventLoopAsync();

    std::filesystem::path lldpDirectory;
    std::vector<std::pair<int, std::string>> links;
    std::map<std::string, std::string> expected;

    SECTION("Single link, single neighbor")
    {
        lldpDirectory = CMAKE_CURRENT_SOURCE_DIR "/tests/files/single-link";
        links = {{8, "ve-image"}};
        expected = {
            {"/neighbors[1]", ""},
            {"/neighbors[1]/ifName", "ve-image"},
            {"/neighbors[1]/remotePortId", "host0"},
            {"/neighbors[1]/remoteSysName", "image"},
            {"/neighbors[1]/remoteChassisId", "7062a9e41c924ac6942da39c56e6b820"},
            {"/neighbors[1]/systemCapabilitiesSupported", "bridge router station-only"},
            {"/neighbors[1]/systemCapabilitiesEnabled", "station-only"},
        };
    }

    SECTION("Two links per one neighbor")
    {
        lldpDirectory = CMAKE_CURRENT_SOURCE_DIR "/tests/files/two-links";
        links = {{3, "enp0s31f6"}, {4, "ve-image"}};
        expected = {
            {"/neighbors[1]", ""},
            {"/neighbors[1]/ifName", "enp0s31f6"},
            {"/neighbors[1]/remoteSysName", "sw-a1128-01.fit.cvut.cz"},
            {"/neighbors[1]/remotePortId", "Gi3/0/7"},
            {"/neighbors[1]/remoteChassisId", "00:b8:b3:e6:17:80"},
            {"/neighbors[1]/systemCapabilitiesSupported", "bridge router"},
            {"/neighbors[1]/systemCapabilitiesEnabled", "bridge"},
            {"/neighbors[2]", ""},
            {"/neighbors[2]/ifName", "ve-image"},
            {"/neighbors[2]/remoteSysName", "image"},
            {"/neighbors[2]/remotePortId", "host0"},
            {"/neighbors[2]/remoteChassisId", "8b90f96f448140fb9b5d9d68e86d052e"},
            {"/neighbors[2]/systemCapabilitiesSupported", "bridge router station-only"},
            {"/neighbors[2]/systemCapabilitiesEnabled", "station-only"},
        };
    }

    SECTION("Multiple neighbors")
    {
        lldpDirectory = CMAKE_CURRENT_SOURCE_DIR "/tests/files/multiple-neighbors";
        links = {{1, "host0"}};
        expected = {
            {"/neighbors[1]", ""},
            {"/neighbors[1]/ifName", "host0"},
            {"/neighbors[1]/remoteChassisId", "1631331c24bb499bb644fcdf7c9fd467"},
            {"/neighbors[1]/remotePortId", "host0"},
            {"/neighbors[1]/remoteSysName", "image"},
            {"/neighbors[1]/systemCapabilitiesEnabled", "station-only"},
            {"/neighbors[1]/systemCapabilitiesSupported", "bridge router station-only"},
            {"/neighbors[2]", ""},
            {"/neighbors[2]/ifName", "host0"},
            {"/neighbors[2]/remoteChassisId", "1efe5cecbfc248a09065ad6177a98b41"},
            {"/neighbors[2]/remotePortId", "vb-image2"},
            {"/neighbors[2]/remoteSysName", "enterprise"},
            {"/neighbors[2]/systemCapabilitiesEnabled", "station-only"},
            {"/neighbors[2]/systemCapabilitiesSupported", "bridge router station-only"},
        };
    }

    auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>(lldpDirectory, sdbus::createSessionBusConnection(), serverBus);
    srSubs->oper_get_items_subscribe("czechlight-lldp", lldp::sysrepo::Callback(lldp), "/czechlight-lldp:nbr-list");

    auto dbusServer = DbusServer(*dbusServerConnection);
    dbusServer.setLinks(links);
    clSess->session_switch_ds(SR_DS_OPERATIONAL);
    REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == expected);
}
