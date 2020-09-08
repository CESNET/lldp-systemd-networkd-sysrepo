#include <sdbus-c++/sdbus-c++.h>
#include "dbus_server.h"

using namespace std::literals;

/** @brief Create a dbus server on the connection */
DbusServer::DbusServer(sdbus::IConnection& connection, sdbus::ObjectPath objectPath, std::string interface)
    : m_manager(sdbus::createObject(connection, objectPath))
{
    // create manager object
    m_manager->registerMethod("ListLinks").onInterface(interface).implementedAs([this]() { return m_links; });
    m_manager->finishRegistration();
}

void DbusServer::setLinks(const std::vector<std::pair<int, std::string>>& links)
{
    m_links.clear();

    for (const auto& [id, name] : links) {
        m_links.push_back(std::make_tuple(id, name, "/some/path/"s + std::to_string(id)));
    }
}