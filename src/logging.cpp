/*
 * Copyright (C) 2016-2018 CESNET, https://photonics.cesnet.cz/
 * Copyright (C) 2020 FIT CVUT, https://fit.cvut.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>, Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

#include "logging.h"

namespace lldp::utils {

/** @short Initialize logging

Creates and registers all required loggers and connect them to the provided sink.
*/
void initLogs([[maybe_unused]] std::shared_ptr<spdlog::sinks::sink> sink)
{

}

} /* namespace lldp::utils */
