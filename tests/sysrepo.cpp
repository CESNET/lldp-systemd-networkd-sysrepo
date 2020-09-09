#include <sysrepo-cpp/Connection.hpp>
#include "sysrepo/Daemon.h"

#include "trompeloeil_doctest.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    TEST_SYSREPO_INIT_LOGS;

    auto srConn = std::make_shared<sysrepo::Connection>("lldp-systemd-networkd-sysrepo");
    auto daemon = std::make_shared<Daemon>(srConn);

    auto conn = std::make_shared<sysrepo::Connection>("test-client");
    auto session = std::make_shared<sysrepo::Session>(conn);

    daemon->ensureYangModule("czechlight-lldp", "2020-08-25");
}