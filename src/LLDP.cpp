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

LLDPDataProvider::~LLDPDataProvider() = default;

/* @brief Lists links using networkd dbus interface and returns them as a list of pairs <link_id, link_name>. */
std::vector<std::pair<int, std::string>> LLDPDataProvider::networkdListLinks()
{
    std::vector<sdbus::Struct<int, std::string, sdbus::ObjectPath>> links;
    auto managerObj = sdbus::createProxy("org.freedesktop.network1", "/org/freedesktop/network1");
    managerObj->callMethod("ListLinks").onInterface("org.freedesktop.network1.Manager").storeResultsTo(links);

    spdlog::debug("Found {} links", links.size());

    std::vector<std::pair<int, std::string>> res;
    std::transform(links.begin(), links.end(), std::back_inserter(res), [](const auto& e) { return std::make_pair(std::get<0>(e), std::get<1>(e)); });
    return res;
}

/* @brief Opens binary file with LLDP data of a link. */
bool LLDPDataProvider::open_lldp_neighbors(int ifindex, FILE** ret)
{
    char* p = NULL;
    FILE* f;

    if (asprintf(&p, "/run/systemd/netif/lldp/%i", ifindex) < 0)
        return false;

    f = fopen(p, "re");
    free(p);
    if (!f)
        return false;

    *ret = f;
    return true;
}

int LLDPDataProvider::next_lldp_neighbor(FILE* f, sd_lldp_neighbor** ret)
{
    void* raw = NULL;
    size_t l;
    size_t u;
    int r;

    assert(f);
    assert(ret);

    l = fread(&u, 1, sizeof(u), f);
    if (l == 0 && feof(f))
        return 0;
    if (l != sizeof(u))
        return -EBADMSG;

    /* each LLDP packet is at most MTU size, but let's allow up to 4KiB just in case */
    if (le64toh(u) >= 4096)
        return -EBADMSG;

    raw = malloc(sizeof(uint8_t) * le64toh(u));
    if (!raw)
        return -ENOMEM;

    if (fread(raw, 1, le64toh(u), f) != le64toh(u))
        return -EBADMSG;

    r = sd_lldp_neighbor_from_raw(ret, raw, le64toh(u));
    free(raw);
    if (r < 0)
        return r;

    return 1;
}

std::vector<NeighbourEntry> LLDPDataProvider::getNeighbours() const
{
    std::vector<NeighbourEntry> res;

    for (const auto& [linkId, linkName] : networkdListLinks()) {
        spdlog::debug("Listing neighbours on network link name={}, id={}", linkId, linkName);
        FILE* f = nullptr;
        sd_lldp_neighbor* n;

        if (!open_lldp_neighbors(linkId, &f)) {
            spdlog::debug("  failed to open LLDP file.");
            continue;
        }

        for (;;) {
            if (next_lldp_neighbor(f, &n) <= 0) {
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