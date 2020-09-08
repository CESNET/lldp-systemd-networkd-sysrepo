#include "LLDP.h"

#include "trompeloeil_doctest.h"
#include "fake.h"
#include "test_log_setup.h"
#include "test_sysrepo_helpers.h"

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
