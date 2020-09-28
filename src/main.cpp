#include <csignal>
#include <spdlog/details/registry.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>
#include <sysrepo-cpp/Session.hpp>

#include "LLDP.h"
#include "callback.h"
#include "logging.h"


volatile int exit_application = 0;
static void
sigint_handler([[maybe_unused]] int signum)
{
    exit_application = 1;
}

int main()
{
    std::shared_ptr<spdlog::sinks::sink> loggingSink;
    if (lldp::utils::isJournaldActive()) {
        loggingSink = lldp::utils::create_journald_sink();
    } else {
        loggingSink = std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    }

    lldp::utils::initLogs(loggingSink);
    spdlog::set_level(spdlog::level::debug);
    spdlog::get("sysrepo")->set_level(spdlog::level::info);

    try {
        sysrepo::S_Connection conn(new sysrepo::Connection("lldp-systemd-networkd-sysrepo")); // FIXME
        sysrepo::S_Session sess(new sysrepo::Session(conn));
        sysrepo::S_Subscribe subscribe(new sysrepo::Subscribe(sess));
        spdlog::debug("Initialized sysrepo connection");

        auto dbusClientConnection = sdbus::createSystemBusConnection();
        spdlog::debug("Initialized dbus connection");

        auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>("/run/systemd/netif/lldp", *dbusClientConnection, "org.freedesktop.network1", "/org/freedesktop/network1", "org.freedesktop.network1.Manager");
        spdlog::debug("Initialized lldp");

        subscribe->dp_get_items_subscribe("/czechlight-lldp:nbr-list", std::make_shared<lldp::sysrepo::Callback>(lldp));
        spdlog::debug("Initialized sysrepo callback");
        spdlog::info("Started");

        signal(SIGINT, sigint_handler);
        while (!exit_application) {
            sleep(1000);
        }

        spdlog::get("main")->debug("Shutting down");
        return 0;
    } catch (std::exception& e) {
        spdlog::error("Exception {}", e.what());
        lldp::utils::fatalException(spdlog::details::registry::instance().default_logger(), e, "main");
    }
}
