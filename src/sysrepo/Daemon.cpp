/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#include <lldp/LLDP.h>
#include <sysrepo-cpp/Session.hpp>
#include <utility>
#include "sysrepo/Daemon.h"
#include "utils/log.h"

namespace lldp::sysrepo {

class OperationalDataCallback : public ::sysrepo::Callback {
public:
    explicit OperationalDataCallback(std::shared_ptr<lldp::LLDPDataProvider> lldpData)
        : m_lldpData(std::move(lldpData))
        , m_lastRequestId(0)
    {
    }

    int dp_get_items(const char* xpath, [[maybe_unused]] ::sysrepo::S_Vals_Holder vals, uint64_t request_id, const char* original_xpath, [[maybe_unused]] void* private_ctx) override
    {
        spdlog::get("main")->debug("dp_get_items: XPath {} req {} orig-XPath {}", xpath, request_id, original_xpath);

        // when asking for something in the subtree of the same request
        if (m_lastRequestId == request_id) {
            spdlog::get("main")->trace(" ops data request already handled");
            return SR_ERR_OK;
        }
        m_lastRequestId = request_id;

        size_t allocatedVals = 0;
        auto out = vals->allocate(allocatedVals);
        size_t valIdx = 0;

        auto filler = [&xpath, &out, &valIdx](const lldp::NeighbourEntry& neighbour, const std::string& key) {
            if (auto it = neighbour.m_properties.find(key); it != neighbour.m_properties.end()) {
                out->val(valIdx)->set(
                    (std::string(xpath) + "/if-name[ifName='" + neighbour.m_portId + "']/" + key).c_str(),
                    it->second.c_str(),
                    SR_STRING_T);
                valIdx += 1;
            }
        };

        for (const auto& neighbour : m_lldpData->getNeighbours()) {
            out = vals->reallocate(allocatedVals += neighbour.m_properties.size());

            filler(neighbour, "remotePortId");
            filler(neighbour, "remoteSysName");
        }

        return SR_ERR_OK;
    }

private:
    std::shared_ptr<lldp::LLDPDataProvider> m_lldpData;
    uint64_t m_lastRequestId;
};

Daemon::Daemon(std::shared_ptr<::sysrepo::Connection> srConnection, const std::string& xpath, const std::string& yangModule, const std::string& yangRevision, std::shared_ptr<lldp::LLDPDataProvider> lldpData)
    : m_conn(std::move(srConnection))
    , m_session(std::make_shared<::sysrepo::Session>(m_conn))
    , m_subscription(std::make_shared<::sysrepo::Subscribe>(m_session))
{
    ensureYangModule(yangModule, yangRevision);
    m_subscription->dp_get_items_subscribe(xpath.c_str(), std::make_shared<OperationalDataCallback>(std::move(lldpData)));
}

Daemon::~Daemon() = default;

/** @short Ensure that the specified YANG module is enabled in sysrepo with a correct revision */
void Daemon::ensureYangModule(const std::string& yang, const std::string& revision)
{
    const auto& schemas = m_session->list_schemas();
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
