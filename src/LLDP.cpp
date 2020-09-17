/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/
#include "LLDP.h"

namespace lldp::lldp {

LLDPDataProvider::~LLDPDataProvider() = default;

std::vector<NeighbourEntry> LLDPDataProvider::getNeighbours() const
{
    return {};
}

std::ostream& operator<<(std::ostream& os, const NeighbourEntry& entry)
{
    os << "lldp::lldp::NeighbourEntry(" << entry.m_portId << ": {";

    for (auto it = entry.m_properties.begin(); it != entry.m_properties.end(); ++it) {
        if (it != entry.m_properties.begin()) {
            os << ", ";
        }

        os << it->first << ": " << it->second;
    }

    return os << "}";
}

} /* namespace lldp::lldp */