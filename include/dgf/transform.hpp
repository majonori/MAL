#pragma once
#include "../common/modint.hpp"
#include "../number_theory/sieve.hpp"
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace mal { namespace dgf {

using coef = mint<998244353>;
using series = std::vector<coef>;

// Index 0 is unused: a.size() == N+1 represents coefficients 1..N.
inline std::vector<int> primes(int n) {
    return number_theory::Sieve(n).primes;
}

// Sum over divisors, respectively sum over multiples. The inverses undo them.
inline void divisor_zeta(series& a) {
    const int n = int(a.size()) - 1;
    for (int p : primes(n)) for (int i = 1; i <= n / p; ++i) a[i * p] += a[i];
}
inline void divisor_mobius(series& a) {
    const int n = int(a.size()) - 1;
    for (int p : primes(n)) for (int i = n / p; i >= 1; --i) a[i * p] -= a[i];
}
inline void multiple_zeta(series& a) {
    const int n = int(a.size()) - 1;
    for (int p : primes(n)) for (int i = n / p; i >= 1; --i) a[i] += a[i * p];
}
inline void multiple_mobius(series& a) {
    const int n = int(a.size()) - 1;
    for (int p : primes(n)) for (int i = 1; i <= n / p; ++i) a[i] -= a[i * p];
}

inline series lcm_convolution(series a, series b) {
    if (a.size() != b.size()) throw std::invalid_argument("dgf: different truncations");
    divisor_zeta(a); divisor_zeta(b);
    for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
    divisor_mobius(a);
    return a;
}
inline series gcd_convolution(series a, series b) {
    if (a.size() != b.size()) throw std::invalid_argument("dgf: different truncations");
    multiple_zeta(a); multiple_zeta(b);
    for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
    multiple_mobius(a);
    return a;
}

namespace detail {
// The interactive library is a separate translation unit. Retain public inline
// functions so a contestant can link using declarations alone.
#define MAL_DGF_KEEP(name) __attribute__((used)) auto name##_kept = &name
MAL_DGF_KEEP(divisor_zeta);
MAL_DGF_KEEP(divisor_mobius);
MAL_DGF_KEEP(multiple_zeta);
MAL_DGF_KEEP(multiple_mobius);
MAL_DGF_KEEP(lcm_convolution);
MAL_DGF_KEEP(gcd_convolution);
#undef MAL_DGF_KEEP
} // namespace detail

} } // namespace mal::dgf
