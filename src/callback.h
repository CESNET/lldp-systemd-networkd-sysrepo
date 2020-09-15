/*
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
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

void ensureYangModule(::sysrepo::S_Session session, const std::string& yang, const std::string& revision);

class Callback : public ::sysrepo::Callback {
public:
    Callback();
    int dp_get_items(const char* xpath, ::sysrepo::S_Vals_Holder vals, uint64_t request_id, const char* original_xpath, void* private_ctx) override;

private:
    uint64_t m_lastRequestId;
};

} /* namespace lldp::sysrepo */