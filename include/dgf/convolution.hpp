#pragma once
#include "fps.hpp"

namespace mal { namespace dgf {

inline int gcd_int(int a, int b) {
    while (b) { int t = a % b; a = b; b = t; }
    return a;
}

// EI's decomposition: ab=n, gcd(a,b)=k, so n=k^2*x and a/k,b/k are coprime.
// For each k, ranked divisor zeta / pointwise polynomial multiplication /
// divisor mobius extracts the coprime convolution at rank omega_distinct(x).
// O(N (log log N)^2) arithmetic, O(N log log N) peak memory.
inline series fast_mul(const series& f, const series& g) {
    pair_check(f, g);
    const int n = int(f.size()) - 1;
    series answer(n + 1);
    for (int k = 1; k <= n / k; ++k) {
        int m = n / (k * k);
        // For short fibers, a direct coprime convolution has much lower overhead.
        if (m <= 256) {
            for (int a = 1; a <= m; ++a)
                for (int b = 1; b <= m / a; ++b)
                    if (gcd_int(a, b) == 1)
                        answer[k * k * a * b] += f[k * a] * g[k * b];
            continue;
        }
        number_theory::Sieve sieve(m);
        const auto& lp = sieve.spf;
        const auto& ps = sieve.primes;
        std::vector<int> rank(m + 1);
        for (int i = 2; i <= m; ++i) {
            int p = lp[i], t = i / p;
            rank[i] = rank[t] + (t % p != 0);
        }
        std::vector<std::size_t> ab_offset(m + 2), c_offset(m + 2);
        for (int i = 1; i <= m; ++i) {
            ab_offset[i + 1] = ab_offset[i] + rank[i] + 1;
            // A divisor y of x may have degree greater than omega(y) after
            // pointwise multiplication (e.g. 2*2 at y=2). Keep that degree
            // until Mobius inversion removes its contribution at x.
            c_offset[i + 1] = c_offset[i] + 2 * rank[i] + 1;
        }
        std::vector<coef> a(ab_offset[m + 1]), b(ab_offset[m + 1]), c(c_offset[m + 1]);
        for (int i = 1; i <= m; ++i) {
            a[ab_offset[i] + rank[i]] = f[k * i];
            b[ab_offset[i] + rank[i]] = g[k * i];
        }
        for (int p : ps) for (int i = 1; i <= m / p; ++i)
            for (int j = 0; j <= rank[i]; ++j) {
                a[ab_offset[i * p] + j] += a[ab_offset[i] + j];
                b[ab_offset[i * p] + j] += b[ab_offset[i] + j];
            }
        for (int i = 1; i <= m; ++i)
            for (int j = 0; j <= rank[i]; ++j)
                for (int t = 0; t <= rank[i]; ++t)
                    c[c_offset[i] + j + t] += a[ab_offset[i] + j] * b[ab_offset[i] + t];
        for (int p : ps) for (int i = m / p; i >= 1; --i)
            for (int j = 0; j <= 2 * rank[i]; ++j)
                c[c_offset[i * p] + j] -= c[c_offset[i] + j];
        for (int i = 1; i <= m; ++i) answer[k * k * i] += c[c_offset[i] + rank[i]];
    }
    return answer;
}

namespace detail {
__attribute__((used)) auto fast_mul_kept = &fast_mul;
} // namespace detail

} } // namespace mal::dgf
