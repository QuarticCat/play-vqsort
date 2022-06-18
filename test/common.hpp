#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <ranges>

#include "catch2/catch_amalgamated.hpp"
#include "pvq/pvq.hpp"

using namespace pvq::detail;

template<class T>
T get_random() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<uint8_t> dis{};
    return dis(gen);
}
