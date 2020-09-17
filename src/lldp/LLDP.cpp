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

} /* namespace lldp::lldp */