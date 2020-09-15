/*
 * Copyright (C) 2016-2018 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>
 *
*/

#pragma once

#include <doctest/doctest.h>
#include <map>
#include <sstream>
#include "lldp/LLDP.h"

namespace doctest {

template <>
struct StringMaker<std::vector<uint8_t>> {
    static String convert(std::vector<uint8_t> const& value)
    {
        std::ostringstream ss;
        trompeloeil::print(ss, value);
        return ss.str().c_str();
    }
};

template <>
struct StringMaker<std::map<std::string, std::string>> {
    static String convert(const std::map<std::string, std::string>& map)
    {
        std::ostringstream os;
        os << "{" << std::endl;
        for (const auto& [key, value] : map) {
            os << "  \"" << key << "\": \"" << value << "\"," << std::endl;
        }
        os << "}";
        return os.str().c_str();
    }
};

}

namespace trompeloeil {
template <>
void print(std::ostream& os, const lldp::lldp::NeighbourEntry& e)
{
    os << "lldp::lldp::NeighbourEntry{" << e.m_portId << ", " << e.m_remotePortId << "}" << std::endl;
}
}

namespace doctest {
template <>
struct StringMaker<lldp::lldp::NeighbourEntry> {
    static String convert(const lldp::lldp::NeighbourEntry& value)
    {
        std::ostringstream ss;
        trompeloeil::print(ss, value);
        return ss.str().c_str();
    }
};
}
