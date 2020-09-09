/*
 * Copyright (C) 2016-2018 CESNET, https://photonics.cesnet.cz/
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
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

namespace lldp::utils {

void initLogs(std::shared_ptr<spdlog::sinks::sink> sink);

} /* namespace lldp::utils */
