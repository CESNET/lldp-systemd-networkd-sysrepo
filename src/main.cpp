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

        /* Let's run forever in an infinite blocking loop. We have originally proposed something like
         * "while(!exit) sleep(big_number);" and expect that sleep(3) will get interrupted by SIGTERM.
         * However, such code is vulnerable to race-conditions. The SIGTERM could be received right after
         * the while condition is evaluated but before the sleep(3) was invoked.
         *
         * This can be solved using pselect and blocking signals.
         * See https://www.linuxprogrammingblog.com/all-about-linux-signals?page=6
         */

        // Install sighandler for SIGTERM
        struct sigaction sigact;
        memset(&sigact, 0, sizeof(sigact));
        sigact.sa_handler = [](int) { g_exit_application = 1; };
        sigact.sa_flags = SA_SIGINFO;
        sigaction(SIGTERM, &sigact, nullptr);

        // Block SIGTERM
        sigset_t sigset, oldset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTERM);
        sigprocmask(SIG_BLOCK, &sigset, &oldset);

        while (!g_exit_application) {
            fd_set fd;
            FD_ZERO(&fd);

            // if SIGTERM received at this point, it is deffered until pselect is entered which enables the signal processing again
            pselect(0, &fd, NULL, NULL, NULL, &oldset);
        }

        spdlog::info("Shutting down");
        return 0;
    } catch (std::exception& e) {
        lldp::utils::fatalException(spdlog::details::registry::instance().default_logger(), e, "main");
    }
}
