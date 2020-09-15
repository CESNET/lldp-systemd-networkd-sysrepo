/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
*/

#pragma once

#include <string>
#include <vector>

namespace lldp::lldp {

struct NeighbourEntry {
    std::string m_portId;
    std::string m_remotePortId;
};

class LLDPDataProvider {
public:
    virtual std::vector<NeighbourEntry> getNeighbours() const;
};

} /* namespace lldp::lldp */
