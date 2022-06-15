#pragma once

#include <immintrin.h>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace pvq {

#ifdef __clang__
    #define PVQ_UNROLL _Pragma("unroll")
#else
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

template<class T>
concept SimdElement = std::is_arithmetic_v<T> && !std::is_const_v<T>;

template<SimdElement T>
using Avx = T __attribute__((vector_size(32)));

template<SimdElement T>
constexpr size_t AVX_LANES = 32 / sizeof(T);

inline bool any(__m256i x) {
    return _mm256_movemask_epi8(x) != 0;
}

inline bool all(__m256i x) {
    return _mm256_movemask_epi8(x) == -1;
}

}  // namespace pvq
