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
    auto dbusClientConnection = sdbus::createSessionBusConnection();
    dbusServerConnection->enterEventLoopAsync();

    spdlog::info("dbus-connection-eventloopasync");

    auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>(CMAKE_CURRENT_SOURCE_DIR "/tests/files/single-link", std::move(dbusClientConnection), serverBus);

    spdlog::info("lldp");

    srSubs->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>(lldp));

    spdlog::info("subscribed");

    auto dbusServer = DbusServer(*dbusServerConnection);
    dbusServer.setLinks({{8, "ve-image"}});

    spdlog::info("server");

    REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                {"/if-name[ifName='ve-image']", ""},
                {"/if-name[ifName='ve-image']/ifName", "ve-image"},
                {"/if-name[ifName='ve-image']/remotePortId", "host0"},
                {"/if-name[ifName='ve-image']/remoteSysName", "image"},
                {"/if-name[ifName='ve-image']/remoteChassisId", "7062a9e41c924ac6942da39c56e6b820"},
                {"/if-name[ifName='ve-image']/systemCapabilitiesSupported", "bridge router station-only"},
                {"/if-name[ifName='ve-image']/systemCapabilitiesEnabled", "station-only"},
            });

    spdlog::info("data");
}
