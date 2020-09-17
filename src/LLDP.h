/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#pragma once

#include <map>
#include <spdlog/fmt/ostr.h> // allow spdlog to use operator<<(ostream, NeighbourEntry)
#include <string>
#include <vector>

namespace lldp::lldp {

struct NeighbourEntry {
    std::string m_portId;
    std::map<std::string, std::string> m_properties;
};
std::ostream& operator<<(std::ostream& os, const NeighbourEntry& entry);


class LLDPDataProvider {
public:
    std::vector<NeighbourEntry> getNeighbours() const;
};

} /* namespace lldp::lldp */
