#pragma once
#include <cstdint>
#include <vector>
#include <complex>

using ll = long long;
using ull = unsigned long long;
using ld = long double;
using cpx = std::complex<double>;
using std::vector;

constexpr double PI = 3.141592653589793238462643383279502884;

inline int glim(std::size_t x) {
    return x <= 1 ? 1 : 2 << (31 ^ __builtin_clz((int)x - 1));
}

/* Edited on 2026/07/27
This is a public definition.
*/
