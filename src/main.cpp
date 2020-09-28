#include <csignal>
#include <spdlog/details/registry.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>
#include <sysrepo-cpp/Session.hpp>
#include <thread>

#include "LLDP.h"
#include "callback.h"
#include "logging.h"

volatile sig_atomic_t g_exit_application = 0;

int main()
{
    std::shared_ptr<spdlog::sinks::sink> loggingSink;
    if (lldp::utils::isJournaldActive()) {
        loggingSink = lldp::utils::create_journald_sink();
    } else {
        loggingSink = std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    }

    lldp::utils::initLogs(loggingSink);
    spdlog::set_level(spdlog::level::info);
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

        // Let's run forever in an infinite loop with a sleep. Receiving a signal wakes the application up.
        struct sigaction sigact;
        memset(&sigact, 0, sizeof(sigact));
        sigact.sa_handler = [](int) { g_exit_application = 1; };
        sigact.sa_flags = SA_SIGINFO;

        sigaction(SIGTERM, &sigact, nullptr);

        while (!g_exit_application) {
            //std::this_thread::sleep_for(std::chrono::seconds(1'000'000)); // FIXME: does not work with sigaction
            sleep(1'000'000);
        }

        spdlog::info("Shutting down");
        return 0;
    } catch (std::exception& e) {
        lldp::utils::fatalException(spdlog::details::registry::instance().default_logger(), e, "main");
    }
}
