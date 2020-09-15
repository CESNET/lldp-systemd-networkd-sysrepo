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

namespace lldp::sysrepo {

class Daemon {
    std::shared_ptr<::sysrepo::Connection> m_conn;
    std::shared_ptr<::sysrepo::Session> m_session;
    std::shared_ptr<::sysrepo::Subscribe> m_subscription;

public:
    Daemon(std::shared_ptr<::sysrepo::Connection> srConnection, const std::string& xpath, const std::string& yangModule, const std::string& yangRevision);
    virtual ~Daemon();

private:
    void ensureYangModule(const std::string& yang, const std::string& revision);
};

} /* namespace lldp::sysrepo */