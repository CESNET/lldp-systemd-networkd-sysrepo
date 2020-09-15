/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>

namespace sysrepo {
class Connection;
class Session;
class Subscribe;
} /* namespace sysrepo */

namespace lldp::lldp {
class LLDPDataProvider;
} /* namespace lldp::lldp */

namespace lldp::sysrepo {

class Daemon {
public:
    explicit Daemon(std::shared_ptr<::sysrepo::Connection> srConnection);
    virtual ~Daemon();

    void ensureYangModule(const std::string& yang, const std::string& revision, const std::optional<std::string>& feature = std::nullopt);
    void subscribe(const std::string& xpath, std::shared_ptr<lldp::LLDPDataProvider> provider);

private:
    std::shared_ptr<::sysrepo::Connection> m_conn;
    std::shared_ptr<::sysrepo::Session> m_session;
    std::shared_ptr<::sysrepo::Subscribe> m_subscription;
};

} /* namespace lldp::sysrepo */