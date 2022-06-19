#include "common.hpp"

TEMPLATE_TEST_CASE("Test sorting network", "[sorting_network]", int8_t, uint32_t, int64_t) {
    SECTION("sort16") {
        auto cols = NETWORK_MAX_ELEMENT_COLS<TestType>;
        auto [data1, data2] = get_random_vec_pair<TestType>(NETWORK_MAX_ELEMENTS<TestType>);

        Avx<TestType> data1_vec[NETWORK_ROWS];
        for (size_t i = 0; i < NETWORK_ROWS; ++i) data1_vec[i] = loadu(data1.data() + i * cols);
        sort16<std::less<>>(data1_vec);
        for (size_t i = 0; i < NETWORK_ROWS; ++i) storeu(data1.data() + i * cols, data1_vec[i]);

        std::vector<std::vector<TestType>> data2_buf(cols);
        for (size_t i = 0; i < data2.size(); ++i) data2_buf[i % cols].push_back(data2[i]);
        for (auto&& arr: data2_buf) std::ranges::sort(arr);
        for (size_t i = 0; i < data2.size(); ++i) data2[i] = data2_buf[i % cols][i / cols];

        REQUIRE(data1 == data2);
    }

    SECTION("merge") {
        size_t cols = GENERATE(2, 4, 8, 16);
        if (NETWORK_ROWS * cols > NETWORK_MAX_ELEMENTS<TestType>) return;
        auto [data1, data2] = get_random_vec_pair<TestType>(NETWORK_ROWS * cols);

        Avx<TestType> data1_vec[NETWORK_ROWS];
        for (size_t i = 0; i < NETWORK_ROWS; ++i) data1_vec[i] = loadu(data1.data() + i * cols);
        sort16<std::less<>>(data1_vec);
        if (cols >= 2) merge<2, std::less<>>(data1_vec);
        if (cols >= 4) merge<4, std::less<>>(data1_vec);
        if (cols >= 8) merge<8, std::less<>>(data1_vec);
        if (cols >= 16) merge<16, std::less<>>(data1_vec);
        for (size_t i = 0; i < NETWORK_ROWS; ++i) storeu(data1.data() + i * cols, data1_vec[i]);

        std::ranges::sort(data2);

        REQUIRE(data1 == data2);
    }

    SECTION("sorting_network") {
        auto [data1, data2] = get_random_vec_pair<TestType>(NETWORK_MAX_ELEMENTS<TestType>);

        sorting_network<std::less<>>(data1.data(), NETWORK_MAX_ELEMENT_COLS<TestType>);

        std::ranges::sort(data2);

        REQUIRE(data1 == data2);
    }
}
