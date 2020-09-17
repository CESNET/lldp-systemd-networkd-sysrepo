/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */
#include "LLDP.h"

namespace lldp::lldp {

std::vector<NeighborEntry> LLDPDataProvider::getNeighbors() const
{
    return {
        NeighborEntry{"dummy1", {{"remotePortId", "dummy2"}}},
    };
}

std::ostream& operator<<(std::ostream& os, const NeighborEntry& entry)
{
    os << "lldp::lldp::NeighborEntry(" << entry.m_portId << ": {";

    for (auto it = entry.m_properties.begin(); it != entry.m_properties.end(); ++it) {
        if (it != entry.m_properties.begin()) {
            os << ", ";
        }

        os << it->first << ": " << it->second;
    }

    return os << "}";
}

} /* namespace lldp::lldp */