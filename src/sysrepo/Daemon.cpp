#include <sysrepo-cpp/Session.hpp>
#include "Daemon.h"

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
