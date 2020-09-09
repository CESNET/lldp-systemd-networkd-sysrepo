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

namespace lldp::sysrepo {

class Callback : public ::sysrepo::Callback {
public:
    Callback();
    int dp_get_items(const char* xpath, ::sysrepo::S_Vals_Holder vals, uint64_t request_id, const char* original_xpath, void* private_ctx) override;

private:
    uint64_t m_lastRequestId;
};

} /* namespace lldp::sysrepo */