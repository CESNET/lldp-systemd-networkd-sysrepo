/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#include <sysrepo-cpp/Session.hpp>
#include <utility>
#include "lldp/LLDP.h"
#include "sysrepo/Daemon.h"
#include "utils/log.h"

namespace lldp::sysrepo {

class OperationalDataCallback : public ::sysrepo::Callback {
public:
    explicit OperationalDataCallback(std::shared_ptr<lldp::LLDPDataProvider> provider)
        : m_dataProvider(std::move(provider))
        , m_lastRequestId(0)
    {
    }

    int dp_get_items([[maybe_unused]] const char* xpath, ::sysrepo::S_Vals_Holder vals, [[maybe_unused]] uint64_t request_id, [[maybe_unused]] const char* original_xpath, [[maybe_unused]] void* private_ctx) override
    {
        spdlog::get("main")->debug("dp_get_items: XPath {} req {} orig-XPath {}", xpath, request_id, original_xpath);

        if (m_lastRequestId == request_id) {
            spdlog::get("main")->trace(" ops data request already handled");
            return SR_ERR_OK;
        }
        m_lastRequestId = request_id;

        size_t sz = 0;
        size_t i = 0;
        auto out = vals->allocate(sz);

        for (const auto& e : m_dataProvider->getNeighbours()) {
            out = vals->reallocate(sz += 1);
            out->val(i)->set(
                (std::string("/czechlight-lldp:nbr-list/if-name[ifName='") + e.m_portId + "']/remotePortId").c_str(),
                e.m_remotePortId.c_str(),
                SR_STRING_T);

            i += 1;
        }

        return SR_ERR_OK;
    }

private:
    std::shared_ptr<lldp::LLDPDataProvider> m_dataProvider;
    uint64_t m_lastRequestId;
};

Daemon::Daemon(std::shared_ptr<::sysrepo::Connection> srConnection)
    : m_conn(std::move(srConnection))
    , m_session(std::make_shared<::sysrepo::Session>(m_conn))
    , m_subscription(std::make_shared<::sysrepo::Subscribe>(m_session))
{
}

Daemon::~Daemon() = default;

/** @short Ensure that the specified YANG module is enabled in sysrepo with a correct revision */
void Daemon::ensureYangModule(const std::string& yang, const std::string& revision, const std::optional<std::string>& feature)
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
        if (feature) {
            for (size_t j = 0; j < schema->enabled_feature_cnt(); ++j) {
                if (schema->enabled_features(j) == *feature) {
                    return;
                }
            }
            throw std::runtime_error("Feature " + *feature + " is not enabled in YANG model " + yang);
        }
        return;
    }
    throw std::runtime_error("Requested YANG model " + yang + " revision " + revision + " not available in sysrepo");
}

void Daemon::subscribe(const std::string& xpath, std::shared_ptr<lldp::LLDPDataProvider> provider)
{
    m_subscription->dp_get_items_subscribe(xpath.c_str(), std::make_shared<OperationalDataCallback>(provider));
}

} /* namespace lldp::sysrepo */
