/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <sysrepo-cpp/Session.hpp>
#include "LLDP.h"

namespace lldp::sysrepo {

class Callback : public ::sysrepo::Callback {
public:
    explicit Callback(std::shared_ptr<lldp::LLDPDataProvider> lldp);
    int dp_get_items(const char* xpath, ::sysrepo::S_Vals_Holder vals, uint64_t request_id, const char* original_xpath, void* private_ctx) override;

private:
    std::shared_ptr<lldp::LLDPDataProvider> m_lldp;
    uint64_t m_lastRequestId;
};

} /* namespace lldp::sysrepo */