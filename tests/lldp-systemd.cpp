#include "LLDP.h"

#include "doctest_integration.h"
#include "test_log_setup.h"

#if 1
TEST_CASE("Connection")
{
    TEST_INIT_LOGS;

    auto dataProvider = std::make_shared<lldp::lldp::LLDPDataProvider>();

    for (const auto& n : dataProvider->getNeighbours()) {
        std::ostringstream oss;
        oss << n;
        spdlog::info("Found neighbour: {}", oss.str());
    }
}

#endif
