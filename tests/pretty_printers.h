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

namespace doctest {

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

template <>
struct StringMaker<std::vector<lldp::lldp::NeighborEntry>> {
    static String convert(const std::vector<lldp::lldp::NeighborEntry>& vec)
    {
        std::ostringstream os;
        os << "[" << std::endl;
        for (const auto& e : vec) {
            os << e;
        }
        os << "]";
        return os.str().c_str();
    }
};
}
