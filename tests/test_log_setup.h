/*
 * Copyright (C) 2016-2018 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>
 *
*/

#pragma once

#include <iostream>
#include <spdlog/sinks/ansicolor_sink.h>
#include "logging.h"

#define TEST_INIT_LOGS                                                              \
    auto test_logger = std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>(); \
    lldp::utils::initLogs(test_logger);                                             \
    spdlog::set_pattern("%S.%e [%t %n %L] %v");                                     \
    spdlog::set_level(spdlog::level::trace);                                        \
    spdlog::get("sysrepo")->set_level(spdlog::level::info);                         \
    trompeloeil::stream_tracer tracer {std::cout};
