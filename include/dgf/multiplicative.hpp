#pragma once
#include "convolution.hpp"

namespace mal { namespace dgf {

// Each n = core[n] * prime_power[n], gcd(core[n],prime_power[n])=1.
// These arrays, and the smallest prime factors, are computed in O(N).
struct PrimePowers {
    number_theory::Sieve sieve;
    std::vector<int> prime_power, core, exponent;
    explicit PrimePowers(int n) : sieve(n), prime_power(n + 1),
                                  core(n + 1), exponent(n + 1) {
        if (n >= 1) core[1] = prime_power[1] = 1;
        for (int i = 2; i <= n; ++i) {
            int p = sieve.spf[i], m = i / p;
            if (m % p == 0) {
                prime_power[i] = prime_power[m] * p;
                core[i] = core[m];
                exponent[i] = exponent[m] + 1;
            } else {
                prime_power[i] = p;
                core[i] = m;
                exponent[i] = 1;
            }
        }
    }
};

inline bool is_multiplicative(const series& f) {
    check(f);
    if (f[1] != coef(1)) return false;
    PrimePowers t(int(f.size()) - 1);
    for (std::size_t i = 2; i < f.size(); ++i)
        if (t.core[i] != 1 && f[i] != f[t.core[i]] * f[t.prime_power[i]])
            return false;
    return true;
}
inline bool is_completely_multiplicative(const series& f) {
    check(f);
    if (f[1] != coef(1)) return false;
    number_theory::Sieve s(int(f.size()) - 1);
    for (std::size_t i = 2; i < f.size(); ++i)
        if (f[i] != f[i / s.spf[i]] * f[s.spf[i]]) return false;
    return true;
}
inline bool is_prime_power_supported(const series& f) {
    check(f);
    if (f[1] != coef(0)) return false;
    PrimePowers t(int(f.size()) - 1);
    for (std::size_t i = 2; i < f.size(); ++i)
        if (t.core[i] != 1 && f[i] != coef(0)) return false;
    return true;
}

// Multiplicative Dirichlet convolution, O(N): Bell-series multiplication on
// prime powers followed by multiplicative extension to the other indices.
inline series mul_multiplicative_both(const series& f, const series& g) {
    pair_check(f, g);
    if (f[1] != coef(1) || g[1] != coef(1))
        throw std::invalid_argument("dgf: multiplicative functions require f(1)=g(1)=1");
    int n = int(f.size()) - 1;
    PrimePowers t(n);
    series h(n + 1); h[1] = 1;
    for (int i = 2; i <= n; ++i) {
        if (t.core[i] != 1) { h[i] = h[t.core[i]] * h[t.prime_power[i]]; continue; }
        int p = t.sieve.spf[i];
        for (int q = 1; ; ) {
            h[i] += f[q] * g[i / q];
            if (q == i) break;
            q *= p;
        }
    }
    return h;
}

// f/g when both are multiplicative, O(N); this also computes a multiplicative
// inverse by taking f to be the Dirichlet identity epsilon.
inline series div_multiplicative_both(const series& f, const series& g) {
    pair_check(f, g);
    if (f[1] != coef(1) || g[1] != coef(1))
        throw std::invalid_argument("dgf: multiplicative functions require f(1)=g(1)=1");
    int n = int(f.size()) - 1;
    PrimePowers t(n);
    series h(n + 1); h[1] = 1;
    for (int i = 2; i <= n; ++i) {
        if (t.core[i] != 1) { h[i] = h[t.core[i]] * h[t.prime_power[i]]; continue; }
        int p = t.sieve.spf[i];
        h[i] = f[i];
        for (int q = p; q <= i; ) {
            h[i] -= g[q] * h[i / q];
            if (q == i) break;
            q *= p;
        }
    }
    return h;
}
inline series inv_multiplicative(const series& f) {
    check(f);
    series identity(f.size()); identity[1] = 1;
    return div_multiplicative_both(identity, f);
}

// log(f) is supported only on prime powers if f is multiplicative.
inline series ln_multiplicative(const series& f) {
    check(f);
    if (f[1] != coef(1)) throw std::invalid_argument("dgf: ln requires f(1)=1");
    int n = int(f.size()) - 1;
    PrimePowers t(n);
    int maxe = 0;
    for (int q = n; q > 1; q >>= 1) ++maxe;
    std::vector<coef> inverse(maxe + 1);
    for (int e = 1; e <= maxe; ++e) inverse[e] = coef(e).inv();
    series h(n + 1);
    for (int i = 2; i <= n; ++i) {
        if (t.core[i] != 1) continue;
        int p = t.sieve.spf[i], e = t.exponent[i];
        coef acc = coef(e) * f[i];
        for (int q = p, j = 1; q < i; q *= p, ++j)
            acc -= coef(j) * h[q] * f[i / q];
        h[i] = acc * inverse[e];
    }
    return h;
}

// exp(f) is multiplicative when f(1)=0 and f vanishes away from prime powers.
inline series exp_prime_powers(const series& f) {
    check(f);
    if (f[1] != coef(0)) throw std::invalid_argument("dgf: exp requires f(1)=0");
    int n = int(f.size()) - 1;
    PrimePowers t(n);
    int maxe = 0;
    for (int q = n; q > 1; q >>= 1) ++maxe;
    std::vector<coef> inverse(maxe + 1);
    for (int e = 1; e <= maxe; ++e) inverse[e] = coef(e).inv();
    series h(n + 1); h[1] = 1;
    for (int i = 2; i <= n; ++i) {
        if (t.core[i] != 1) { h[i] = h[t.core[i]] * h[t.prime_power[i]]; continue; }
        int p = t.sieve.spf[i], e = t.exponent[i];
        coef acc = 0;
        for (int q = p, j = 1; ; q *= p, ++j) {
            acc += coef(j) * f[q] * h[i / q];
            if (q == i) break;
        }
        h[i] = acc * inverse[e];
    }
    return h;
}

namespace detail {
#define MAL_DGF_KEEP(name) __attribute__((used)) auto name##_kept = &name
MAL_DGF_KEEP(is_multiplicative);
MAL_DGF_KEEP(is_completely_multiplicative);
MAL_DGF_KEEP(is_prime_power_supported);
MAL_DGF_KEEP(mul_multiplicative_both);
MAL_DGF_KEEP(div_multiplicative_both);
MAL_DGF_KEEP(inv_multiplicative);
MAL_DGF_KEEP(ln_multiplicative);
MAL_DGF_KEEP(exp_prime_powers);
#undef MAL_DGF_KEEP
} // namespace detail

} } // namespace mal::dgf
