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

int Callback::dp_get_items(const char* xpath, [[maybe_unused]] ::sysrepo::S_Vals_Holder vals, uint64_t request_id, const char* original_xpath, [[maybe_unused]] void* private_ctx)
{
    spdlog::debug("dp_get_items: XPath {} req {} orig-XPath {}", xpath, request_id, original_xpath);

    // when asking for something in the subtree of THIS request
    if (m_lastRequestId == request_id) {
        spdlog::trace(" ops data request already handled");
        return SR_ERR_OK;
    }
    m_lastRequestId = request_id;

    // get number of entries
    spdlog::debug("callback: before getNeighbours");
    auto neighbors = m_lldp->getNeighbors();
    spdlog::debug("callback: after getNeighbours");
    size_t allocSize = std::accumulate(neighbors.begin(), neighbors.end(), 0, [](size_t acc, lldp::NeighborEntry elem) { return acc + elem.m_properties.size(); });
    size_t i = 0;
    auto out = vals->reallocate(allocSize);

    spdlog::debug("callback: before loop over neighbours");
    for (const auto& n : neighbors) {
        for (const auto& [key, val] : n.m_properties) { // garbage properties in, garbage out

            auto srType = SR_STRING_T;
            if (key == "systemCapabilitiesSupported" || key == "systemCapabilitiesEnabled") {
                srType = SR_BITS_T;
            }

            out->val(i++)->set(
                (std::string(xpath) + "/if-name[ifName='" + n.m_portId + "']/" + key).c_str(),
                val.c_str(),
                srType);
        }
    }

    return SR_ERR_OK;
}

} /* namespace lldp::sysrepo */
