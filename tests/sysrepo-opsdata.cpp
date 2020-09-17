#include <sysrepo-cpp/Connection.hpp>
#include "callback.h"
#include "LLDP.h"

#include "trompeloeil_doctest.h"
#include "fake.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    trompeloeil::sequence seq1;
    auto lldp = std::make_shared<FakeDataProvider>();

    TEST_INIT_LOGS;
    TEST_INIT_SYSREPO;

    srSubs->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>());

    lldp::sysrepo::ensureYangModule(clSess, "czechlight-lldp", "2020-08-25");

    auto lst = {
        lldp::lldp::NeighbourEntry {"eth0", {{"remotePortId", "rem0"}}},
        lldp::lldp::NeighbourEntry {"eth1", {{"remotePortId", "rem0"}, {"remoteSysName", "n2"}}},
    };
    FAKE_NEIGHBOURS(lst);

    CHECK(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string>({
              {"/if-name[ifName='eth0']", ""},
              {"/if-name[ifName='eth0']/ifName", "eth0"},
              {"/if-name[ifName='eth0']/remotePortId", "rem0"},

              {"/if-name[ifName='eth1']", ""},
              {"/if-name[ifName='eth1']/ifName", "eth1"},
              {"/if-name[ifName='eth1']/remotePortId", "rem0"},
              {"/if-name[ifName='eth1']/remoteSysName", "n2"},
          }));
}
