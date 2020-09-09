#include <sysrepo-cpp/Connection.hpp>
#include "callback.h"

#include "doctest_integration.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    TEST_INIT_LOGS;
    TEST_INIT_SYSREPO;

    lldp::sysrepo::ensureYangModule(srSess, "czechlight-lldp", "2020-08-25");
    srSubs->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>());

    REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                {"/if-name[ifName='dummy1']", ""},
                {"/if-name[ifName='dummy1']/ifName", "dummy1"},
                {"/if-name[ifName='dummy1']/remotePortId", "dummy2"},
            });
}
