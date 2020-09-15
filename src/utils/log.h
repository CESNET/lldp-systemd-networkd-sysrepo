/*
 * Copyright (C) 2016-2018 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>
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