//
// Created by tomas on 15.9.20.
//

#include "LLDP.h"

namespace lldp::lldp {

LLDPDataProvider::~LLDPDataProvider() = default;

std::vector<NeighbourEntry> LLDPDataProvider::getNeighbours() const
{
    return {};
}

} /* namespace lldp::lldp */