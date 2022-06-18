#include "common.hpp"

TEST_CASE("Test sorting_network", "[sorting_network]") {
    std::vector<uint32_t> data1(NETWORK_MAX_ELEMENTS<uint32_t>);
    std::ranges::generate(data1, get_random<uint32_t>);
    std::vector<uint32_t> data2(data1);

    std::sort(data1.begin(), data1.end());
    sorting_network<std::less<>>(data2.data(), NETWORK_MAX_ELEMENT_COLS<uint32_t>);

    REQUIRE(data1 == data2);
}
