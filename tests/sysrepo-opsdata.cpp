#include <sysrepo-cpp/Connection.hpp>
#include "sysrepo/Daemon.h"

#include "trompeloeil_doctest.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    TEST_SYSREPO_INIT_LOGS;
    trompeloeil::sequence seq1;

    auto srConn = std::make_shared<sysrepo::Connection>("lldp-systemd-networkd-sysrepo");
    auto daemon = std::make_shared<lldp::sysrepo::Daemon>(srConn, "/czechlight-lldp:nbr-list", "czechlight-lldp", "2020-08-25");

    auto conn = std::make_shared<sysrepo::Connection>("test-client");
    auto session = std::make_shared<sysrepo::Session>(conn);

    REQUIRE(true); // TODO
}
