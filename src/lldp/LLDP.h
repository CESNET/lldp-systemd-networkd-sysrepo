/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#pragma once

#include <map>
#include <string>
#include <vector>
#include "lldp/NeighbourEntry.h"

namespace lldp::lldp {

class LLDPDataProvider {
public:
    virtual ~LLDPDataProvider();
    virtual std::vector<NeighbourEntry> getNeighbours() const;
};

} /* namespace lldp::lldp */
