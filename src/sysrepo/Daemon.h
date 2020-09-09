#ifndef __DAEMON_H__
#define __DAEMON_H__

#include <memory>
#include <optional>

namespace sysrepo {
class Connection;
class Session;
class Subscribe;
class Callback;
}

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

#endif /* __DAEMON_H__ */