/*
 * Copyright (C) 2020 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>, Tomáš Pecka <tomas.pecka@fit.cvut.cz>
 *
 */

extern "C" {
#include <sysrepo.h>
}
#include <cinttypes>
#include <cstdio>
#include <cxxabi.h>
#include <spdlog/sinks/systemd_sink.h>
#include <sys/stat.h>
#include <unistd.h>
#include "logging.h"

extern "C" {

/** @short Propagate sysrepo events to spdlog */
static void spdlog_sr_log_cb(sr_log_level_t level, const char* message)
{
    // Thread safety note: this is, as far as I know, thread safe:
    // - the static initialization itself is OK
    // - all loggers which we instantiate are thread-safe
    // - std::shared_ptr::operator-> is const, and all const members of that class are documented to be thread-safe
    static auto log = spdlog::get("sysrepo");
    assert(log);
    switch (level) {
    case SR_LL_NONE:
    case SR_LL_ERR:
        log->error(message);
        break;
    case SR_LL_WRN:
        log->warn(message);
        break;
    case SR_LL_INF:
        log->info(message);
        break;
    case SR_LL_DBG:
        log->debug(message);
        break;
    }
}
}

namespace lldp::utils {

/** @short Initialize logging

Creates and registers all required loggers and connect them to the provided sink.
*/
void initLogs(std::shared_ptr<spdlog::sinks::sink> sink)
{
    auto defaultLogger = std::make_shared<spdlog::logger>("lldp-systemd-sysrepo", sink);
    spdlog::register_logger(defaultLogger);
    spdlog::set_default_logger(defaultLogger);

    spdlog::register_logger(std::make_shared<spdlog::logger>("sysrepo", sink));
    sr_log_set_cb(spdlog_sr_log_cb);
}

/** @short Is stderr connected to journald? Not thread safe. */
bool isJournaldActive()
{
    const auto stream = ::getenv("JOURNAL_STREAM");
    if (!stream) {
        return false;
    }
    uintmax_t dev;
    uintmax_t inode;
    if (::sscanf(stream, "%" SCNuMAX ":%" SCNuMAX, &dev, &inode) != 2) {
        return false;
    }
    struct stat buf;
    if (fstat(STDERR_FILENO, &buf)) {
        return false;
    }
    return static_cast<uintmax_t>(buf.st_dev) == dev && static_cast<uintmax_t>(buf.st_ino) == inode;
}

namespace impl {
/** @short Provide better levels, see https://github.com/gabime/spdlog/pull/1292#discussion_r340777258 */
template <typename Mutex>
class journald_sink : public spdlog::sinks::systemd_sink<Mutex> {
public:
    journald_sink()
    {
        this->syslog_levels_ = {/* spdlog::level::trace      */ LOG_DEBUG,
                                /* spdlog::level::debug      */ LOG_INFO,
                                /* spdlog::level::info       */ LOG_NOTICE,
                                /* spdlog::level::warn       */ LOG_WARNING,
                                /* spdlog::level::err        */ LOG_ERR,
                                /* spdlog::level::critical   */ LOG_CRIT,
                                /* spdlog::level::off        */ LOG_ALERT};
    }
};
}

std::shared_ptr<spdlog::sinks::sink> create_journald_sink()
{
    return std::make_shared<impl::journald_sink<std::mutex>>();
}

/** @short Log that everything is screwed up and rethrow

The purpose is to make sure that a nicely formatted error message gets stored into the journald buffer with a high enough priority.
*/
void fatalException [[noreturn]] (std::shared_ptr<spdlog::logger> log, const std::exception& e, const std::string& when)
{
    int demangled;
    char* classname = __cxxabiv1::__cxa_demangle(typeid(e).name(), nullptr, 0, &demangled);
    log->critical("Fatal error in {}: {}", when, demangled == 0 ? classname : typeid(e).name());
    log->critical("{}", e.what());
    free(classname);
    throw;
}

} /* namespace lldp::utils */
