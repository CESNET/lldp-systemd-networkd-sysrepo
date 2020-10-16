/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#include <numeric>
#include "callback.h"
#include "logging.h"

namespace lldp::sysrepo {

Callback::Callback(std::shared_ptr<lldp::LLDPDataProvider> lldp)
    : m_lldp(std::move(lldp))
    , m_lastRequestId(0)
{
}

int Callback::operator()(std::shared_ptr<::sysrepo::Session> session, const char* module_name, const char* xpath, const char* request_xpath, uint32_t request_id, std::shared_ptr<libyang::Data_Node>& parent)
{
    spdlog::debug("operational data callback: XPath {} req {} orig-XPath {}", xpath, request_id, request_xpath);

    // when asking for something in the subtree of THIS request
    if (m_lastRequestId == request_id) {
        spdlog::trace(" ops data request already handled");
        return SR_ERR_OK;
    }
    m_lastRequestId = request_id;

    auto ctx = session->get_context();
    auto mod = ctx->get_module(module_name);

    parent = std::make_shared<libyang::Data_Node>(ctx, "/czechlight-lldp:nbr-list", nullptr, LYD_ANYDATA_CONSTSTRING, 0);

    auto ifc = std::make_shared<libyang::Data_Node>(parent, mod, "if-name");

    for (const auto& n : m_lldp->getNeighbors()) {
        auto key = std::make_shared<libyang::Data_Node>(ifc, mod, "ifName", n.m_portId.c_str());

        for (const auto& [key, val] : n.m_properties) { // garbage properties in, garbage out
            auto prop = std::make_shared<libyang::Data_Node>(ifc, mod, key.c_str(), val.c_str());
        }
    }

    return SR_ERR_OK;
}

} /* namespace lldp::sysrepo */
