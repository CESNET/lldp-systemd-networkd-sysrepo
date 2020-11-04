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

    SECTION("Single link, single neighbour")
    {
        auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>(CMAKE_CURRENT_SOURCE_DIR "/tests/files/single-link", sdbus::createSessionBusConnection(), serverBus);
        srSubs->oper_get_items_subscribe("czechlight-lldp", lldp::sysrepo::Callback(lldp), "/czechlight-lldp:nbr-list");

        auto dbusServer = DbusServer(*dbusServerConnection);
        dbusServer.setLinks({{8, "ve-image"}});

        clSess->session_switch_ds(SR_DS_OPERATIONAL);
        REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                    {"/if-name[ifName='ve-image']", ""},
                    {"/if-name[ifName='ve-image']/ifName", "ve-image"},
                    {"/if-name[ifName='ve-image']/remotePortId", "host0"},
                    {"/if-name[ifName='ve-image']/remoteSysName", "image"},
                    {"/if-name[ifName='ve-image']/remoteChassisId", "7062a9e41c924ac6942da39c56e6b820"},
                    {"/if-name[ifName='ve-image']/systemCapabilitiesSupported", "bridge router station-only"},
                    {"/if-name[ifName='ve-image']/systemCapabilitiesEnabled", "station-only"},
                });
    }

    SECTION("Two links per one neighbours")
    {
        auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>(CMAKE_CURRENT_SOURCE_DIR "/tests/files/two-links", sdbus::createSessionBusConnection(), serverBus);
        srSubs->oper_get_items_subscribe("czechlight-lldp", lldp::sysrepo::Callback(lldp), "/czechlight-lldp:nbr-list");

        auto dbusServer = DbusServer(*dbusServerConnection);
        dbusServer.setLinks({{3, "enp0s31f6"}, {4, "ve-image"}});

        clSess->session_switch_ds(SR_DS_OPERATIONAL);
        REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                    {"/if-name[ifName='enp0s31f6']", ""},
                    {"/if-name[ifName='enp0s31f6']/ifName", "enp0s31f6"},
                    {"/if-name[ifName='enp0s31f6']/remoteSysName", "sw-a1128-01.fit.cvut.cz"},
                    {"/if-name[ifName='enp0s31f6']/remotePortId", "Gi3/0/7"},
                    {"/if-name[ifName='enp0s31f6']/remoteChassisId", "00:b8:b3:e6:17:80"},
                    {"/if-name[ifName='enp0s31f6']/systemCapabilitiesSupported", "bridge router"},
                    {"/if-name[ifName='enp0s31f6']/systemCapabilitiesEnabled", "bridge"},
                    {"/if-name[ifName='ve-image']", ""},
                    {"/if-name[ifName='ve-image']/ifName", "ve-image"},
                    {"/if-name[ifName='ve-image']/remoteSysName", "image"},
                    {"/if-name[ifName='ve-image']/remotePortId", "host0"},
                    {"/if-name[ifName='ve-image']/remoteChassisId", "8b90f96f448140fb9b5d9d68e86d052e"},
                    {"/if-name[ifName='ve-image']/systemCapabilitiesSupported", "bridge router station-only"},
                    {"/if-name[ifName='ve-image']/systemCapabilitiesEnabled", "station-only"},
                });
    }
}
