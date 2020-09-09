/*
 * Copyright (C) 2016-2019 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>
 *
*/

#include "doctest_integration.h"
#include <map>
#include <sysrepo-cpp/Session.hpp>

#define TEST_INIT_SYSREPO                                                                 \
    auto srConn = std::make_shared<sysrepo::Connection>("lldp-systemd-networkd-sysrepo"); \
    auto srSess = std::make_shared<sysrepo::Session>(srConn);                             \
    auto srSubs = std::make_shared<sysrepo::Subscribe>(srSess);                           \
    auto clConn = std::make_shared<sysrepo::Connection>("test-client");                   \
    auto clSess = std::make_shared<sysrepo::Session>(clConn);

/** @short Returns true if str ends with a given suffix */
bool endsWith(const std::string& str, const std::string& suffix)
{
    if (suffix.size() > str.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

/** @short Return a subtree from sysrepo, compacting the XPath */
auto dataFromSysrepo(const std::shared_ptr<sysrepo::Session>& session, const std::string& xpath)
{
    std::map<std::string, std::string> res;
    auto vals = session->get_items((xpath + "//*").c_str());
    REQUIRE(!!vals);
    for (size_t i = 0; i < vals->val_cnt(); ++i) {
        const auto& v = vals->val(i);
        const auto briefXPath = std::string(v->xpath()).substr(endsWith(xpath, ":*") ? xpath.size() - 1 : xpath.size());
        res.emplace(briefXPath, v->val_to_string());
    }
    return res;
}
