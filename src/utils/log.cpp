/*
 * Copyright (C) 2016-2018 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>
 *
*/

#include "utils/log.h"

namespace lldp::utils {

/** @short Initialize logging

Creates and registers all required loggers and connect them to the provided sink.
*/
void initLogs(std::shared_ptr<spdlog::sinks::sink> sink)
{
    for (const auto& name : std::vector<std::string> {"main", "sysrepo"}) {
        spdlog::register_logger(std::make_shared<spdlog::logger>(name, sink));
    }
}

} /* namespace lldp::utils */