#pragma once

#include <immintrin.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace pvq {

template<class T>
concept SimdElement = std::is_arithmetic_v<T> && !std::is_const_v<T>;

template<SimdElement T>
using Avx = T __attribute__((vector_size(32)));

template<SimdElement T>
constexpr size_t AVX_LANES = 32 / sizeof(T);

namespace detail {

#if defined(__clang__)
    #define PVQ_UNROLL _Pragma("unroll")
#elif defined(__GNUC__)
    #define PVQ_UNROLL _Pragma("GCC ivdep")
#endif

template<size_t Start, size_t End, size_t Step = 1>
void unroll(auto&& f) {
    ([&]<size_t... Is>(std::index_sequence<Is...>) {
        (f.template operator()<Is>(), ...);
    }([]<size_t... Is>(std::index_sequence<Is...>) {
        return std::index_sequence<Start + Is * Step...>{};
    }(std::make_index_sequence<End - Start>{})));
}

template<size_t End>
void unroll(auto&& f) {
    unroll<0, End>(f);
}

template<size_t Start, size_t End, size_t Step = 1>
void expand(auto&& f) {
    ([&]<size_t... Is>(std::index_sequence<Is...>) {
        return f.template operator()<Is...>();
    }([]<size_t... Is>(std::index_sequence<Is...>) {
        return std::index_sequence<Start + Is * Step...>{};
    }(std::make_index_sequence<End - Start>{})));
}

template<size_t End>
void expand(auto&& f) {
    expand<0, End>(f);
}

inline void assume(bool pred) {
    assert(pred);

#if defined(__clang__)
    __builtin_assume(pred);
#elif defined(__GNUC__)
    if (!pred) __builtin_unreachable();
#endif
}

template<SimdElement T>
Avx<T> loadu(T* ptr) {
    if constexpr (std::is_same_v<T, float>) {
        return _mm256_loadu_ps(ptr);
    } else if constexpr (std::is_same_v<T, double>) {
        return _mm256_loadu_pd(ptr);
    } else {
        return _mm256_loadu_si256((__m256i*)ptr);
    }
}

template<SimdElement T>
void storeu(T* ptr, Avx<T> x) {
    if constexpr (std::is_same_v<T, float>) {
        _mm256_storeu_ps(ptr, x);
    } else if constexpr (std::is_same_v<T, double>) {
        _mm256_storeu_pd(ptr, x);
    } else {
        _mm256_storeu_si256((__m256i*)ptr, x);
    }
}

inline bool any(__m256i x) {
    return _mm256_movemask_epi8(x) != 0;
}

inline bool all(__m256i x) {
    return _mm256_movemask_epi8(x) == -1;
}

}  // namespace detail

}  // namespace pvq
