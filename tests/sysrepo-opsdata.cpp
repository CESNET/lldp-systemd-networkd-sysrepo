#include <sysrepo-cpp/Connection.hpp>
#include "sysrepo/Daemon.h"

#include "trompeloeil_doctest.h"
#include "fake.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    TEST_SYSREPO_INIT_LOGS;
    trompeloeil::sequence seq1;

    auto srConn = std::make_shared<sysrepo::Connection>("lldp-systemd-networkd-sysrepo");
    auto daemon = std::make_shared<lldp::sysrepo::Daemon>(srConn);

    auto conn = std::make_shared<sysrepo::Connection>("test-client");
    auto session = std::make_shared<sysrepo::Session>(conn);

    auto dataProvider = std::make_shared<FakeDataProvider>();

    daemon->ensureYangModule("czechlight-lldp", "2020-08-25");
    daemon->subscribe("/czechlight-lldp:nbr-list", dataProvider);

    auto lst = {
        lldp::lldp::NeighbourEntry {"eth0", "rem0"},
        lldp::lldp::NeighbourEntry {"eth1", "rem0"},
    };
    FAKE_NEIGHBOURS(lst);

    CHECK(dataFromSysrepo(session, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string>({
              {"/if-name[ifName='eth0']", ""},
              {"/if-name[ifName='eth0']/ifName", "eth0"},
              {"/if-name[ifName='eth0']/remotePortId", "rem0"},

              {"/if-name[ifName='eth1']", ""},
              {"/if-name[ifName='eth1']/ifName", "eth1"},
              {"/if-name[ifName='eth1']/remotePortId", "rem0"},
          }));
}
