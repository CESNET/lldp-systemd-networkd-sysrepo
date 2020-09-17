/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/

#pragma once

#include <trompeloeil.hpp>
#include "LLDP.h"

class FakeDataProvider : public lldp::lldp::LLDPDataProvider {
public:
    MAKE_CONST_MOCK0(getNeighbours, std::vector<lldp::lldp::NeighbourEntry>(), override);
};

#define FAKE_NEIGHBOURS(ENTRIES) REQUIRE_CALL(*lldp, getNeighbours()).IN_SEQUENCE(seq1).RETURN(std::vector<lldp::lldp::NeighbourEntry>(ENTRIES))
