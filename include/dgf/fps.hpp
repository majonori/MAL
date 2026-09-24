#pragma once
#include "transform.hpp"
#include <climits>

namespace mal { namespace dgf {

inline void check(const series& a) {
    if (a.size() < 2 || a[0] != coef(0))
        throw std::invalid_argument("dgf: expected [0,f(1),...,f(N)]");
}
inline void pair_check(const series& a, const series& b) {
    check(a); check(b);
    if (a.size() != b.size()) throw std::invalid_argument("dgf: different truncations");
}

// General Dirichlet convolution. O(N log N), including nonmultiplicative input.
inline series mul(const series& a, const series& b) {
    pair_check(a, b);
    int n = int(a.size()) - 1;
    series c(n + 1);
    for (int i = 1; i <= n; ++i)
        if (a[i] != coef(0))
            for (int j = 1; j <= n / i; ++j) c[i * j] += a[i] * b[j];
    return c;
}

// g(1)=1 and g multiplicative; each prime is processed using its Bell series.
// O(N log log N). Neither condition is tested by this routine.
inline series mul_multiplicative(const series& f, const series& g) {
    pair_check(f, g);
    if (g[1] != coef(1)) throw std::invalid_argument("dgf: g(1) must be 1");
    int n = int(f.size()) - 1;
    series h = f;
    for (int p : primes(n))
        for (int t = n / p; t >= 1; --t)
            for (int q = p; q <= n / t; ) {
                h[t * q] += h[t] * g[q];
                if (q > n / t / p) break;
                q *= p;
            }
    return h;
}

inline std::vector<int> omega(int n) {
    number_theory::Sieve sieve(n);
    std::vector<int> w(n + 1);
    for (int i = 2; i <= n; ++i) {
        w[i] = w[i / sieve.spf[i]] + 1;
    }
    return w;
}

inline series derivative(const series& f) {
    check(f);
    series h = f;
    auto w = omega(int(f.size()) - 1);
    for (std::size_t i = 2; i < f.size(); ++i) h[i] *= coef(w[i]);
    h[1] = 0;
    return h;
}
inline series integral(const series& f, coef constant = 0) {
    check(f);
    if (f[1] != coef(0)) throw std::invalid_argument("dgf: derivative has coefficient 1 = 0");
    series h = f;
    auto w = omega(int(f.size()) - 1);
    int maxw = 0;
    for (int x : w) maxw = std::max(maxw, x);
    std::vector<coef> inverse(maxw + 1);
    for (int i = 1; i <= maxw; ++i) inverse[i] = coef(i).inv();
    for (std::size_t i = 2; i < f.size(); ++i) h[i] *= inverse[w[i]];
    h[1] = constant;
    return h;
}

// f(1) != 0. Recurrence h(n)=(delta(n)-sum_{a>1} f(a)h(n/a))/f(1).
inline series inv(const series& f) {
    check(f);
    if (f[1] == coef(0)) throw std::invalid_argument("dgf: inverse requires f(1) != 0");
    int n = int(f.size()) - 1;
    series h(n + 1), acc(n + 1);
    coef inv1 = f[1].inv();
    for (int i = 1; i <= n; ++i) {
        h[i] = (coef(i == 1) - acc[i]) * inv1;
        for (int k = 2; k <= n / i; ++k) acc[i * k] += h[i] * f[k];
    }
    return h;
}

// Division by a unit, with the active recurrence; O(N log N).
inline series div(const series& f, const series& g) {
    pair_check(f, g);
    if (g[1] == coef(0)) throw std::invalid_argument("dgf: division requires g(1) != 0");
    int n = int(f.size()) - 1;
    series h(n + 1), acc(n + 1);
    coef inv1 = g[1].inv();
    for (int i = 1; i <= n; ++i) {
        h[i] = (f[i] - acc[i]) * inv1;
        for (int k = 2; k <= n / i; ++k) acc[i * k] += h[i] * g[k];
    }
    return h;
}

inline series ln(const series& f) {
    check(f);
    if (f[1] != coef(1)) throw std::invalid_argument("dgf: ln requires f(1) = 1");
    return integral(div(derivative(f), f));
}

// Formal exp of f(1)=0. In characteristic 998244353, requires Omega(n)<MOD.
inline series exp(const series& f) {
    check(f);
    if (f[1] != coef(0)) throw std::invalid_argument("dgf: exp requires f(1) = 0");
    int n = int(f.size()) - 1;
    auto w = omega(n);
    int maxw = 0;
    for (int x : w) maxw = std::max(maxw, x);
    std::vector<coef> inverse(maxw + 1);
    for (int i = 1; i <= maxw; ++i) inverse[i] = coef(i).inv();
    series h(n + 1), acc(n + 1);
    h[1] = 1;
    for (int i = 1; i <= n; ++i) {
        if (i > 1) h[i] = acc[i] * inverse[w[i]];
        for (int k = 2; k <= n / i; ++k) acc[i * k] += h[i] * f[k] * coef(w[k]);
    }
    return h;
}

// Formal scalar power of a unit with f(1)=1 (allows fractional exponents).
inline series pow_unit(const series& f, coef k) {
    series h = ln(f);
    for (std::size_t i = 2; i < h.size(); ++i) h[i] *= k;
    return exp(h);
}

// Integer powers, including nonunit f (k>=0) and negative powers (f(1)!=0).
inline series pow_int(const series& f, long long k) {
    check(f);
    int n = int(f.size()) - 1;
    series one(n + 1); one[1] = 1;
    if (!k) return one;
    if (f[1] != coef(0)) {
        coef c = k < 0 ? f[1].inv().pow(-(k + 1)) * f[1].inv() : f[1].pow(k);
        series u = f;
        coef ci = f[1].inv();
        for (int i = 1; i <= n; ++i) u[i] *= ci;
        series h = pow_unit(u, coef(k));
        for (int i = 1; i <= n; ++i) h[i] *= c;
        return h;
    }
    if (k < 0) throw std::invalid_argument("dgf: negative power of nonunit");
    int least = 2;
    while (least <= n && f[least] == coef(0)) ++least;
    if (least > n || k > 63 || (k >= 1 &&
        [&]() { unsigned long long t = 1; for (int i = 0; i < k; ++i) {
            if (t > (unsigned long long)n / least) return true;
            t *= least;
        } return false; }())) return series(n + 1);
    series base = f;
    for (; k; k >>= 1) {
        if (k & 1) one = mul(one, base);
        if (k > 1) base = mul(base, base);
    }
    return one;
}

// p[0]+p[1]f+..., f(1)=0. Only O(log N) powers can be nonzero.
inline series compose(const series& f, const std::vector<coef>& p) {
    check(f);
    if (f[1] != coef(0)) throw std::invalid_argument("dgf: compose requires f(1) = 0");
    int n = int(f.size()) - 1;
    series h(n + 1), power(n + 1); power[1] = 1;
    for (std::size_t j = 0; j < p.size(); ++j) {
        if (j && (j >= 63 || (1ULL << j) > (unsigned)n)) break;
        if (j) power = mul(power, f);
        if (p[j] != coef(0)) for (int i = 1; i <= n; ++i) h[i] += power[i] * p[j];
    }
    return h;
}

// 1+f+...+f^k. Binary doubling works even when f(1)=1.
inline series geometric_sum(const series& f, unsigned long long k) {
    check(f);
    int n = int(f.size()) - 1;
    series one(n + 1); one[1] = 1;
    series power = one, sum(n + 1), base = f, block = one;
    unsigned long long count = k + 1;
    if (count == 0) throw std::overflow_error("dgf: k+1 overflow");
    while (count) {
        if (count & 1) {
            auto v = mul(power, block);
            for (int i = 1; i <= n; ++i) sum[i] += v[i];
            power = mul(power, base);
        }
        count >>= 1;
        if (count) {
            auto v = mul(base, block);
            for (int i = 1; i <= n; ++i) block[i] += v[i];
            base = mul(base, base);
        }
    }
    return sum;
}

// Solve f=g*h up to h(N) when g(1)=0. f and g must be known through d*N,
// where d is the smallest index with g(d)!=0. Optional verification checks
// every coefficient of f through d*N; without it existence is assumed.
inline series generalized_div(const series& f, const series& g, int n, bool verify = false) {
    check(f); check(g);
    if (n < 1 || f.size() != g.size() || g[1] != coef(0))
        throw std::invalid_argument("dgf: generalized division input");
    int d = 2;
    while (d < int(g.size()) && g[d] == coef(0)) ++d;
    if (d == int(g.size()) || 1LL * d * n >= (long long)g.size())
        throw std::invalid_argument("dgf: need coefficients through d*N");
    int bound = d * n;
    series h(n + 1), acc(bound + 1);
    coef invd = g[d].inv();
    for (int i = 1; i <= n; ++i) {
        h[i] = (f[d * i] - acc[d * i]) * invd;
        for (int j = d + 1; j <= bound / i; ++j) acc[i * j] += h[i] * g[j];
    }
    if (verify) {
        // Recheck with a direct product: coefficients at indices not divisible
        // by d are also constraints on whether the quotient exists.
        series product(bound + 1);
        for (int i = 1; i <= n; ++i)
            for (int j = d; j <= bound / i; ++j) product[i * j] += h[i] * g[j];
        for (int i = 1; i <= bound; ++i)
            if (product[i] != f[i]) throw std::invalid_argument("dgf: quotient does not exist");
    }
    return h;
}

namespace detail {
#define MAL_DGF_KEEP(name) __attribute__((used)) auto name##_kept = &name
MAL_DGF_KEEP(mul);
MAL_DGF_KEEP(mul_multiplicative);
MAL_DGF_KEEP(derivative);
MAL_DGF_KEEP(integral);
MAL_DGF_KEEP(inv);
MAL_DGF_KEEP(div);
MAL_DGF_KEEP(ln);
MAL_DGF_KEEP(exp);
MAL_DGF_KEEP(pow_unit);
MAL_DGF_KEEP(pow_int);
MAL_DGF_KEEP(compose);
MAL_DGF_KEEP(geometric_sum);
MAL_DGF_KEEP(generalized_div);
#undef MAL_DGF_KEEP
} // namespace detail

} } // namespace mal::dgf
