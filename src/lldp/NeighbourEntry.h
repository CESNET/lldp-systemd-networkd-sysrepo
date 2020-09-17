/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#pragma once

#include <map>
#include <spdlog/fmt/ostr.h> // allow spdlog to use operator<<(ostream, NeighbourEntry)
#include <string>

namespace lldp::lldp {

struct NeighbourEntry {
    std::string m_portId;
    std::map<std::string, std::string> m_properties;
};

std::ostream& operator<<(std::ostream& os, const NeighbourEntry& entry);

} /* namespace lldp::lldp */