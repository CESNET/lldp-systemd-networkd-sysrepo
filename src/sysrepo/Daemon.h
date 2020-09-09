/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#pragma once

#include <memory>
#include <optional>

namespace sysrepo {
class Connection;
class Session;
class Subscribe;
}

namespace lldp::sysrepo {

class Daemon {
public:
    Daemon(std::shared_ptr<::sysrepo::Connection> srConnection);
    virtual ~Daemon();

    void ensureYangModule(const std::string& yang, const std::string& revision, const std::optional<std::string>& feature = std::nullopt);

private:
    std::shared_ptr<::sysrepo::Connection> m_conn;
    std::shared_ptr<::sysrepo::Session> m_session;
    std::shared_ptr<::sysrepo::Subscribe> m_subscription;
};

} /* namespace lldp::sysrepo */