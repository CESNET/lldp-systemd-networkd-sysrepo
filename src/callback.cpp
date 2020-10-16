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

int Callback::oper_get_items(::sysrepo::S_Session session, const char* module_name, const char* xpath, const char* request_xpath, uint32_t request_id, libyang::S_Data_Node& parent, [[maybe_unused]] void* private_data)
{
    spdlog::debug("oper_get_items: XPath {} req {} orig-XPath {}", xpath, request_id, request_xpath);

    // when asking for something in the subtree of THIS request
    if (m_lastRequestId == request_id) {
        spdlog::trace(" ops data request already handled");
        return SR_ERR_OK;
    }
    m_lastRequestId = request_id;

    libyang::S_Context ctx = session->get_context();
    libyang::S_Module mod = ctx->get_module(module_name);

    parent.reset(new libyang::Data_Node(ctx, "/czechlight-lldp:nbr-list", nullptr, LYD_ANYDATA_CONSTSTRING, 0));

    libyang::S_Data_Node ifc(new libyang::Data_Node(parent, mod, "if-name"));

    for (const auto& n : m_lldp->getNeighbors()) {
        libyang::S_Data_Node key(new libyang::Data_Node(ifc, mod, "ifName", n.m_portId.c_str()));

        for (const auto& [key, val] : n.m_properties) { // garbage properties in, garbage out
            libyang::S_Data_Node prop(new libyang::Data_Node(ifc, mod, key.c_str(), val.c_str()));
        }
    }

    return SR_ERR_OK;
}

} /* namespace lldp::sysrepo */
