#pragma once
#include "dirichlet_power.hpp"

namespace mal { namespace dgf {

// Forward prime-prefix cancellation: sum_{p<=x} P(p), x in D(N).
// Basic O(deg(P) * N^(3/4)/log N) implementation (no accelerated
// N^(2/3) forward data structure); O(deg(P)*sqrt N) storage.
inline BlockPrefix min25_prime_polynomial(const QuotientGrid& grid,
                                           const std::vector<coef>& polynomial) {
    if (polynomial.empty() || polynomial.size() + 1 >= 998244353ULL)
        throw std::invalid_argument("Min25: empty polynomial or degree too high");
    const std::int64_t n = grid.limit();
    BlockPrefix result(grid);
    number_theory::Sieve sieve(grid.root());
    for (std::size_t k = 0; k < polynomial.size(); ++k) {
        if (polynomial[k] == coef(0)) continue;
        BlockPrefix g(grid);
        for (std::size_t i = 0; i < g.size(); ++i) {
            std::int64_t x = grid.values()[i];
            g[i] = power_sum(x, int(k)) - coef(1);
        }
        for (int p : sieve.primes) {
            std::int64_t square = 1LL * p * p;
            if (square > n) break;
            coef prime_power = coef(p).pow(k);
            coef previous = g.at(p - 1);
            // Read g(x/p) as it stood at the end of the previous prime:
            // x/p < p^2 <= x, so descending order is essential.
            for (std::size_t i = g.size(); i-- > 0; ) {
                std::int64_t x = grid.values()[i];
                if (x < square) break;
                g[i] -= prime_power * (g.at(x / p) - previous);
            }
        }
        for (std::size_t i = 0; i < g.size(); ++i)
            result[i] += polynomial[k] * g[i];
    }
    return result;
}

// Ordinary prime-prefix Min25 followed by Dirichlet ln/exp.
// Restricted to multiplicative f with f(p)=P(p), and arbitrary f(p^e).
// This is NOT the reverse-Euler-product generalized sieve, which handles
// factors 1+A_i with support beyond prime powers (including nonmultiplicative f).
// The exp below uses O(log N) block convolutions, not Zak's optimized exp.
template<class Bell>
inline BlockPrefix bell_via_prime_prefix_hybrid(const QuotientGrid& grid,
                                                const std::vector<coef>& polynomial,
                                                Bell bell, int cut) {
    if (cut < 0 || cut > grid.root())
        throw std::invalid_argument("Min25: prime cutoff outside [0,sqrt(N)]");
    BlockPrefix q = min25_prime_polynomial(grid, polynomial);
    number_theory::Sieve sieve(grid.root());
    std::vector<std::pair<std::int64_t, coef>> events;
    std::vector<EulerFactor> small;
    for (int p : sieve.primes) {
        if (p > grid.root()) break;
        int emax = 0;
        for (std::int64_t power = p; power <= grid.limit(); ) {
            ++emax;
            if (power > grid.limit() / p) break;
            power *= p;
        }
        std::vector<coef> a(emax + 1), loga(emax + 1);
        for (int e = 1; e <= emax; ++e) a[e] = bell(p, e);
        if (a[1] != polynomial_at(polynomial, p))
            throw std::invalid_argument("Min25: Bell f(p) disagrees with polynomial");
        if (p <= cut) {
            EulerFactor factor;
            std::int64_t power = 1;
            for (int e = 1; e <= emax; ++e) {
                power *= p;
                if (a[e] != coef(0)) factor.terms.emplace_back(power, a[e]);
            }
            // A factor with f(p)=0 may start at p^2 or later.
            if (!factor.terms.empty() && factor.terms.front().first <= cut) {
                small.push_back(std::move(factor));
                events.emplace_back(p, coef(0) - a[1]);
                continue;
            }
        }
        for (int e = 1; e <= emax; ++e) {
            coef v = coef(e) * a[e];
            for (int j = 1; j < e; ++j) v -= coef(j) * loga[j] * a[e - j];
            loga[e] = v * coef(e).inv();
        }
        std::int64_t power = p;
        for (int e = 2; e <= emax; ++e) {
            power *= p;
            events.emplace_back(power, loga[e]);
        }
    }
    std::sort(events.begin(), events.end(), [](const std::pair<std::int64_t, coef>& a,
                                               const std::pair<std::int64_t, coef>& b) {
        return a.first < b.first;
    });
    coef extra = 0;
    std::size_t j = 0;
    for (std::size_t i = 0; i < q.size(); ++i) {
        std::int64_t x = grid.values()[i];
        while (j < events.size() && events[j].first <= x) extra += events[j++].second;
        q[i] += extra;
    }
    BlockPrefix large(grid);
    if (cut == grid.root()) {
        // Every remaining support exceeds sqrt(N), hence q*q vanishes.
        for (std::size_t i = 0; i < q.size(); ++i) large[i] = coef(1) + q[i];
    } else {
        large = block_exp(q);
    }
    if (small.empty()) return large;
    std::sort(small.begin(), small.end(), [](const EulerFactor& a, const EulerFactor& b) {
        return a.terms.front().first < b.terms.front().first;
    });
    return reverse_euler_product(large, small, cut + 1);
}

template<class Bell>
inline BlockPrefix bell_via_prime_prefix_exp(const QuotientGrid& grid,
                                             const std::vector<coef>& polynomial,
                                             Bell bell, int dense_limit) {
    if (dense_limit < grid.root() || dense_limit > grid.limit())
        throw std::invalid_argument("Min25: dense limit must cover sqrt(N)");
    return bell_via_prime_prefix_hybrid(grid, polynomial, bell, 0);
}

// Compatibility name retained for callers of the first release.
template<class Bell>
inline BlockPrefix min25_bell(const QuotientGrid& grid,
                             const std::vector<coef>& polynomial,
                             Bell bell, int dense_limit) {
    return bell_via_prime_prefix_exp(grid, polynomial, bell, dense_limit);
}

} } // namespace mal::dgf
