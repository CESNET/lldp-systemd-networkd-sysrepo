/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>, Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#pragma once

#include <memory>
#include <spdlog/spdlog.h>

/** @file
  * @short Implementation of initialization of logging
*/


namespace spdlog {
class logger;
}

namespace lldp::utils {
void initLogs(std::shared_ptr<spdlog::sinks::sink> sink);
bool isJournaldActive();
std::shared_ptr<spdlog::sinks::sink> create_journald_sink();
void fatalException [[noreturn]] (std::shared_ptr<spdlog::logger> log, const std::exception& e, const std::string& when);
} /* namespace lldp::utils */
