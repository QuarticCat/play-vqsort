#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <ranges>
#include <tuple>

#include "catch2/catch_amalgamated.hpp"
#include "pvq/pvq.hpp"

using namespace pvq::detail;

template<class T>
T get_random() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<uint8_t> dis{};
    return dis(gen);
}

template<class T>
std::tuple<std::vector<T>, std::vector<T>> get_random_vec_pair(size_t n) {
    std::vector<T> data1(n);
    std::ranges::generate(data1, get_random<T>);
    std::vector<T> data2(data1);
    // for simd memory access safety
    data1.reserve(data1.size() + AVX_LANES<T>);
    data2.reserve(data2.size() + AVX_LANES<T>);
    return {std::move(data1), std::move(data2)};
}
