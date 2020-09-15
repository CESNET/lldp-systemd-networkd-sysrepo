#include <sysrepo-cpp/Connection.hpp>
#include "callback.h"

#include "trompeloeil_doctest.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    trompeloeil::sequence seq1;

    TEST_INIT_LOGS;
    TEST_INIT_SYSREPO;

    srSubs->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>());

    lldp::sysrepo::ensureYangModule(clSess, "czechlight-lldp", "2020-08-25");
    REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                {"/if-name[ifName='eth0']", ""},
                {"/if-name[ifName='eth0']/ifName", "dummy1"},
                {"/if-name[ifName='eth0']/remotePortId", "dummy2"},
            });
}
