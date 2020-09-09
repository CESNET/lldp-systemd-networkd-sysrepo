#include <sysrepo-cpp/Connection.hpp>
#include "callback.h"

#include "doctest_integration.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

TEST_CASE("Connection")
{
    TEST_INIT_LOGS;
    TEST_INIT_SYSREPO;

    srSubs->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>());

    REQUIRE(dataFromSysrepo(clSess, "/czechlight-lldp:nbr-list") == std::map<std::string, std::string> {
                {"/if-name[ifName='dummy1']", ""},
                {"/if-name[ifName='dummy1']/ifName", "dummy1"},
                {"/if-name[ifName='dummy1']/remotePortId", "dummy2"},
            });
}
