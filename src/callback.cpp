/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

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

    // TODO: Fill vals with data.
    auto out = vals->allocate(1);
    out->val(0)->set("/czechlight-lldp:nbr-list/if-name[ifName='dummy1']/remotePortId", "dummy2", SR_STRING_T);

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
