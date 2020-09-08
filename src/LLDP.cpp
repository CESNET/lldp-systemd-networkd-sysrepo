/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
*/
#include <sdbus-c++/sdbus-c++.h>
#include <spdlog/spdlog.h>
#include "LLDP.h"

namespace lldp::lldp {

/* @brief Lists links using networkd dbus interface and returns them as a list of pairs <link_id, link_name>. */
std::vector<std::pair<int, std::string>> LLDPDataProvider::listLinks()
{
    std::vector<sdbus::Struct<int, std::string, sdbus::ObjectPath>> links;
    std::vector<std::pair<int, std::string>> res; // we only want to return pairs <id, name>, not full sdbus struct

    sdbus::createProxy("org.freedesktop.network1", "/org/freedesktop/network1")->callMethod("ListLinks").onInterface("org.freedesktop.network1.Manager").storeResultsTo(links);

    std::transform(links.begin(), links.end(), std::back_inserter(res), [](const auto& e) { return std::make_pair(std::get<0>(e), std::get<1>(e)); });
    return res;
}

/* @brief Opens binary file with LLDP data of a link.
 *
 * Inspired systemd's networkctl code.
 * */
bool LLDPDataProvider::nextNeighbour(FILE* f, sd_lldp_neighbor** ret)
{
    assert(f);
    assert(ret);

    // read neighbour size in bytes
    size_t rawSz;
    size_t rd = fread(&rawSz, 1, sizeof(rawSz), f);

    // check we read correct amount of bytes, also each LLDP packet is at most MTU size, but let's allow up to 4KiB just in case
    if ((rd == 0 && feof(f)) || rd != sizeof(rawSz) || le64toh(rawSz) >= 4096) {
        return false;
    }

    // read raw data
    auto* raw = new uint8_t[le64toh(rawSz)];
    if (fread(raw, 1, le64toh(rawSz), f) != le64toh(rawSz)) {
        return false;
    }

    // let systemd parse from raw
    int r = sd_lldp_neighbor_from_raw(ret, raw, le64toh(rawSz));
    delete[] raw;
    return r >= 0;
}

std::vector<NeighbourEntry> LLDPDataProvider::getNeighbours() const
{
    std::vector<NeighbourEntry> res;

    for (const auto& [linkId, linkName] : listLinks()) {
        spdlog::debug("LLDP: Collecting neighbours on '{}' (id {})", linkName, linkId);

        // open lldp datafile
        std::string lldpFilename = "/run/systemd/netif/lldp/" + std::to_string(linkId);
        FILE* f = fopen(lldpFilename.c_str(), "re");
        if (!f) {
            spdlog::debug("  failed to open LLDP datafile ({}).", lldpFilename);
            continue;
        }

        for (;;) {
            sd_lldp_neighbor* n = nullptr;
            if (nextNeighbour(f, &n) <= 0) {
                break;
            }

            const char *system_name = nullptr, *port_id = nullptr;
            (void)sd_lldp_neighbor_get_system_name(n, &system_name);
            (void)sd_lldp_neighbor_get_port_id_as_string(n, &port_id);

            res.push_back(NeighbourEntry {linkName, {
                                                        {"remoteSysName", system_name},
                                                        {"remotePortId", port_id},
                                                    }});
            sd_lldp_neighbor_unrefp(&n);
        }

        fclose(f);
    }

    return res;
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