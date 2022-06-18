#pragma once

#include <memory>

#include "common.hpp"

namespace pvq::detail {

constexpr size_t NETWORK_ROWS = 16;
constexpr size_t NETWORK_MAX_COLS = 16;

template<SimdElement T>
constexpr size_t NETWORK_MAX_ELEMENT_COLS = std::min(AVX_LANES<T>, NETWORK_MAX_COLS);
template<SimdElement T>
constexpr size_t NETWORK_MAX_ELEMENTS = NETWORK_ROWS* NETWORK_MAX_ELEMENT_COLS<T>;

/// Compare and swap
template<class Comp, SimdElement T>
[[gnu::always_inline]] void minmax(Avx<T>& a, Avx<T>& b) {
    auto c = a;
    a = Comp{}(a, b) ? a : b;
    b = Comp{}(c, b) ? b : c;
}

/// Green's irregular sorting network
template<class Comp, SimdElement T>
[[gnu::always_inline]] void sort16(Avx<T> (&v)[16]) {
    for (size_t range = 2; range <= 16; range *= 2) {
        for (size_t group = 0; group < 16; group += range) {
            for (size_t i = group; i < group + range / 2; ++i) {
                minmax<Comp>(v[i], v[i + range / 2]);
            }
        }
    }
    // This part has no particular pattern
    minmax<Comp>(v[0x5], v[0xa]);
    minmax<Comp>(v[0x6], v[0x9]);
    minmax<Comp>(v[0x3], v[0xc]);
    minmax<Comp>(v[0x7], v[0xb]);
    minmax<Comp>(v[0xd], v[0xe]);
    minmax<Comp>(v[0x4], v[0x8]);
    minmax<Comp>(v[0x1], v[0x2]);
    minmax<Comp>(v[0x1], v[0x4]);
    minmax<Comp>(v[0x7], v[0xd]);
    minmax<Comp>(v[0x2], v[0x8]);
    minmax<Comp>(v[0xb], v[0xe]);
    minmax<Comp>(v[0x2], v[0x4]);
    minmax<Comp>(v[0x5], v[0x6]);
    minmax<Comp>(v[0x9], v[0xa]);
    minmax<Comp>(v[0xb], v[0xd]);
    minmax<Comp>(v[0x3], v[0x8]);
    minmax<Comp>(v[0x7], v[0xc]);
    minmax<Comp>(v[0x3], v[0x5]);
    minmax<Comp>(v[0x6], v[0x8]);
    minmax<Comp>(v[0x7], v[0x9]);
    minmax<Comp>(v[0xa], v[0xc]);
    minmax<Comp>(v[0x3], v[0x4]);
    minmax<Comp>(v[0x5], v[0x6]);
    minmax<Comp>(v[0x7], v[0x8]);
    minmax<Comp>(v[0x9], v[0xa]);
    minmax<Comp>(v[0xb], v[0xc]);
    minmax<Comp>(v[0x6], v[0x7]);
    minmax<Comp>(v[0x8], v[0x9]);
}

/// Reverse every N-element group
template<size_t N, SimdElement T>
[[gnu::always_inline]] Avx<T> reverse_groups(Avx<T> v) {
    Avx<T> ret;
    for (size_t group = 0; group < AVX_LANES<T>; group += N) {
        for (size_t i = group; i < group + N; ++i) {
            ret[i] = v[(AVX_LANES<T> + group) - (i - group) - 1];
        }
    }
    return ret;
}

/// Rotate every N-element group by N/2
template<size_t N, SimdElement T>
[[gnu::always_inline]] Avx<T> rotate_groups(Avx<T> v) {
    Avx<T> ret;
    for (size_t group = 0; group < AVX_LANES<T>; group += N) {
        for (size_t i = group; i < group + N / 2; ++i) ret[i] = v[i + N / 2];
        for (size_t i = group + N / 2; i < group + N; ++i) ret[i] = v[i - N / 2];
    }
    return ret;
}

/// Select every (N/2)-element group alternatively
template<size_t N, SimdElement T>
[[gnu::always_inline]] Avx<T> odd_even_select(Avx<T> a, Avx<T> b) {
    Avx<T> ret;
    for (size_t group = 0; group < AVX_LANES<T>; group += N) {
        for (size_t i = group; i < group + N / 2; ++i) ret[i] = a[i];
        for (size_t i = group + N / 2; i < group + N; ++i) ret[i] = b[i];
    }
    return ret;
}

/// Sort lane pairs such like 0 <-> 3, 1 <-> 2 within every N-element group
template<size_t N, class Comp, SimdElement T>
[[gnu::always_inline]] Avx<T> sort_pairs_reverse(Avx<T> v) {
    auto u = reverse_groups<N>(v);
    minmax<Comp>(v, u);
    return odd_even_select<N>(v, u);
}

/// Sort lane pairs such like 0 <-> 2, 1 <-> 3 within every N-element group
template<size_t N, class Comp, SimdElement T>
[[gnu::always_inline]] Avx<T> sort_pairs_distance(Avx<T> v) {
    auto u = rotate_groups<N>(v);
    minmax<Comp>(v, u);
    return odd_even_select<N>(v, u);
}

/// Bitonic merge
template<size_t N, class Comp, SimdElement T>
[[gnu::always_inline]] void merge(Avx<T> (&v)[16]) {
    for (size_t range = 16; range >= 2; range /= 2) {
        for (size_t group = 0; group < 16; group += range) {
            for (size_t i = group + range / 2; i < group + range; ++i) {
                v[i] = reverse_groups<N>(v[i]);
            }
            for (size_t i = group; i < group + range / 2; ++i) {
                minmax<Comp>(v[i], v[(range + group) - (i - group) - 1]);
            }
        }
    }
    for (size_t i = 0; i < 16; ++i) sort_pairs_reverse<N, Comp>(v[i]);
    if constexpr (N >= 16)
        for (size_t i = 0; i < 16; ++i) sort_pairs_distance<4, Comp>(v[i]);
    if constexpr (N >= 8)
        for (size_t i = 0; i < 16; ++i) sort_pairs_distance<2, Comp>(v[i]);
    if constexpr (N >= 4)
        for (size_t i = 0; i < 16; ++i) sort_pairs_distance<1, Comp>(v[i]);
}

template<class Comp, SimdElement T>
[[gnu::always_inline]] void sorting_network(T* __restrict buf, size_t cols) {
    assume(cols <= NETWORK_MAX_ELEMENT_COLS<T>);

    // TODO: bypass buffer
    // TODO: interleave memory accesses and operations

    Avx<T> v[16];
    for (size_t i = 0; i < 16; ++i) v[i] = loadu(buf + i * cols);

    sort16<Comp>(v);

    if (cols >= 2) [[likely]]
        merge<2, Comp>(v);
    if (cols >= 4) [[likely]]
        merge<4, Comp>(v);
    if (cols >= 8) [[likely]]
        merge<8, Comp>(v);
    if (cols >= 16) [[likely]]
        merge<16, Comp>(v);

    for (size_t i = 0; i < 16; ++i) storeu(buf + i * cols, v[i]);
}

}  // namespace pvq::detail
