#pragma once

#include <memory>

#include "common.hpp"

namespace pvq::detail {

template<class Comp, SimdElement T>
[[gnu::always_inline]] void sorting_network(/* aligned */ T* __restrict buf, size_t cols) {}

}  // namespace pvq::detail
