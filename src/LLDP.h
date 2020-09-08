/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#pragma once

#include <fstream>
#include <filesystem>
#include <map>
#include <sdbus-c++/sdbus-c++.h>
#include <spdlog/fmt/ostr.h> // allow spdlog to use operator<<(ostream, NeighborEntry)
#include <string>
#include <systemd/sd-lldp.h>
#include <vector>

namespace lldp::lldp {

struct NeighborEntry {
    std::string m_portId;
    std::map<std::string, std::string> m_properties;
};
std::ostream& operator<<(std::ostream& os, const NeighborEntry& entry);

/** @brief sd_lldp_neighbor requires deletion by invoking sd_lldp_neighbor_unrefp */
struct sd_lldp_neighbor_deleter {
    void operator()(sd_lldp_neighbor* e) const;
};
using sd_lldp_neighbor_managed = std::unique_ptr<sd_lldp_neighbor, sd_lldp_neighbor_deleter>;

class LLDPDataProvider {
public:
    LLDPDataProvider(std::filesystem::path dataDirectory, sdbus::IConnection& dbusConnection, const std::string& dbusNetworkdBus, const std::string& dbusNetworkdObject, const std::string& dbusNetworkdInterface);
    std::vector<NeighborEntry> getNeighbors() const;

private:
    std::filesystem::path m_dataDirectory;
    std::unique_ptr<sdbus::IProxy> m_networkdDbusProxy;
    std::string m_networkdDbusInterface;

    std::vector<std::pair<int, std::string>> listLinks() const;
    static sd_lldp_neighbor_managed nextNeighbor(std::ifstream& ifs);
};

} /* namespace lldp::lldp */
