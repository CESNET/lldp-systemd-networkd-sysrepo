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
#include <systemd/sd-lldp.h>
#include <vector>

namespace lldp::lldp {

struct NeighbourEntry {
    std::string m_portId;
    std::map<std::string, std::string> m_properties;
};
std::ostream& operator<<(std::ostream& os, const NeighbourEntry& entry);


class LLDPDataProvider {
public:
    std::vector<NeighbourEntry> getNeighbours() const;

private:
    static std::vector<std::pair<int, std::string>> listLinks();
    static bool nextNeighbour(FILE* f, sd_lldp_neighbor** ret);
};

} /* namespace lldp::lldp */
