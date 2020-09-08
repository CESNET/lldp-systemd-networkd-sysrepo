#include <sysrepo-cpp/Connection.hpp>
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

    // Use the real paths but prefixed with "local." or "/local"
    const std::string bus = "local.org.freedesktop.network1",
                      object = "/local/org/freedesktop/network1",
                      interface = "local.org.freedesktop.network1.Manager";

    auto dbusServerConnection = sdbus::createSessionBusConnection(bus);
    auto dbusClientConnection = sdbus::createSessionBusConnection();
    dbusServerConnection->enterEventLoopAsync();

    auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>(CMAKE_CURRENT_SOURCE_DIR "/tests/files/lldp1", *dbusClientConnection, bus, object, interface);
    srSubs->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>(lldp));

    auto dbusServer = DbusServer(*dbusServerConnection, object, interface);
    dbusServer.setLinks({{8, "ve-image"}});

    REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                {"/if-name[ifName='ve-image']", ""},
                {"/if-name[ifName='ve-image']/ifName", "ve-image"},
                {"/if-name[ifName='ve-image']/remotePortId", "host0"},
                {"/if-name[ifName='ve-image']/remoteSysName", "image"},
                {"/if-name[ifName='ve-image']/remoteChassisId", "7062a9e41c924ac6942da39c56e6b820"},

            });

    dbusServerConnection->leaveEventLoop();
}
