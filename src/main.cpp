#include <csignal>
#include <docopt/docopt.h>
#include <spdlog/details/registry.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/spdlog.h>
#include <sysrepo-cpp/Session.hpp>
#include <thread>

#include "LLDP.h"
#include "LLDP_SYSTEMD_NETWORKD_SYSREPO_VERSION.h"
#include "callback.h"
#include "logging.h"

/** @short Extract log level from a CLI option */
spdlog::level::level_enum parseLogLevel(const std::string& name, const docopt::value& option)
{
    long x;
    try {
        x = option.asLong();
    } catch (std::invalid_argument&) {
        throw std::runtime_error(name + " log level: expecting integer");
    }
    static_assert(spdlog::level::trace < spdlog::level::off, "spdlog::level levels have changed");
    static_assert(spdlog::level::off == 6, "spdlog::level levels have changed");
    if (x < 0 || x > 5)
        throw std::runtime_error(name + " log level invalid or out-of-range");
    return static_cast<spdlog::level::level_enum>(5 - x);
}

static const char usage[] =
    R"(Control CzechLight devices through NETCONF and Sysrepo

Usage:
  lldp-systemd-networkd-sysrepod
    [--log-level=<Level>]
    [--sysrepo-log-level=<Level>]
  lldp-systemd-networkd-sysrepod (-h | --help)
  lldp-systemd-networkd-sysrepod --version

Options:
  -h --help                         Show this screen.
  --version                         Show version.
  --log-level=<N>                   Log level for everything [default: 3]
  --sysrepo-log-level=<N>           Log level for the sysrepo library [default: 3]
)";

volatile sig_atomic_t g_exit_application = 0;

int main(int argc, char* argv[])
{
    std::shared_ptr<spdlog::sinks::sink> loggingSink;
    if (lldp::utils::isJournaldActive()) {
        loggingSink = lldp::utils::create_journald_sink();
    } else {
        loggingSink = std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    }

    auto args = docopt::docopt(usage,
                               {argv + 1, argv + argc},
                               true,
                               "lldp-systemd-networkd-sysrepod " LLDP_SYSTEMD_NETWORKD_SYSREPO_VERSION,
                               true);

    lldp::utils::initLogs(loggingSink);
    spdlog::set_level(parseLogLevel("Generic", args["--log-level"]));
    spdlog::get("sysrepo")->set_level(parseLogLevel("Sysrepo library", args["--sysrepo-log-level"]));

    try {
        sysrepo::S_Connection conn(new sysrepo::Connection());
        sysrepo::S_Session sess(new sysrepo::Session(conn));
        sysrepo::S_Subscribe subscribe(new sysrepo::Subscribe(sess));
        spdlog::debug("Initialized sysrepo connection");

        auto dbusClientConnection = sdbus::createSystemBusConnection();
        spdlog::debug("Initialized dbus connection");

        auto lldp = std::make_shared<lldp::lldp::LLDPDataProvider>("/run/systemd/netif/lldp", std::move(dbusClientConnection), "org.freedesktop.network1");
        spdlog::debug("Initialized lldp");

        subscribe->oper_get_items_subscribe("czechlight-lldp", lldp::sysrepo::Callback(lldp), "/czechlight-lldp:nbr-list");
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
