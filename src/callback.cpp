/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#include <numeric>
#include "LLDP.h"
#include "callback.h"
#include "logging.h"

namespace lldp::sysrepo {

Callback::Callback()
    : m_lastRequestId(0)
{
}

int Callback::dp_get_items(const char* xpath, [[maybe_unused]] ::sysrepo::S_Vals_Holder vals, uint64_t request_id, const char* original_xpath, [[maybe_unused]] void* private_ctx)
{
    spdlog::debug("dp_get_items: XPath {} req {} orig-XPath {}", xpath, request_id, original_xpath);

    // when asking for something in the subtree of THIS request
    if (m_lastRequestId == request_id) {
        spdlog::trace(" ops data request already handled");
        return SR_ERR_OK;
    }
    m_lastRequestId = request_id;

    lldp::LLDPDataProvider lldp;

    // get number of entries
    auto neighbours = lldp.getNeighbours();
    size_t allocSize = std::accumulate(neighbours.begin(), neighbours.end(), 0, [](size_t acc, lldp::NeighbourEntry elem) { return acc + elem.m_properties.size(); });
    size_t i = 0;
    auto out = vals->reallocate(allocSize);

    for (const auto& n : neighbours) {
        for (const auto& [key, val] : n.m_properties) { // garbage properties in, garbage out
            out->val(i++)->set(
                (std::string(xpath) + "/if-name[ifName='" + n.m_portId + "']/" + key).c_str(),
                val.c_str(),
                SR_STRING_T);
        }
    }

    return SR_ERR_OK;
}

/** @short Ensure that the specified YANG module is enabled in sysrepo with a correct revision */
void ensureYangModule(::sysrepo::S_Session session, const std::string& yang, const std::string& revision)
{
    const auto& schemas = session->list_schemas();
    for (size_t i = 0; i < schemas->schema_cnt(); ++i) {
        const auto& schema = schemas->schema(i);
        if (schema->module_name() != yang) {
            continue;
        }
        if (revision != schema->revision()->revision()) {
            continue;
        }
        if (!schema->implemented()) {
            throw std::runtime_error("Requested YANG model " + yang + " is not implemented in sysrepo");
        }
        return;
    }
    throw std::runtime_error("Requested YANG model " + yang + " revision " + revision + " not available in sysrepo");
}

} /* namespace lldp::sysrepo */
