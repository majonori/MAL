#pragma once
#include "dirichlet_power.hpp"

namespace mal { namespace dgf {

// Independent ln-exp route to the prime prefix sum of p^k.
// Let D_k(n)=n^k and Q_k=ln_*(D_k). Then Q_k(p^e)=p^(ke)/e.
// Compute Q_0 on n<=sqrt(N), set H=exp_*(Q_0), and use one block
// division: D_k/H=exp_*(Q_k-Q_0)=epsilon+Q_k-Q_0, since the tail
// is supported above sqrt(N). Subtract the prime-power terms e>=2.
// No forward Min25 prime cancellation is performed here.
inline BlockPrefix prime_monomial_via_ln_division(const QuotientGrid& grid,
                                                   int degree) {
    if (degree < 0 || degree + 2 >= 998244353)
        throw std::invalid_argument("ln-exp: invalid monomial degree");
    number_theory::Sieve sieve(grid.root());
    std::vector<coef> small_coeff(grid.root() + 1);
    std::vector<std::pair<std::int64_t, coef>> prime_powers;
    for (int p : sieve.primes) {
        std::int64_t power = p;
        coef weighted = coef(p).pow(degree);
        for (int e = 1; ; ++e) {
            coef value = weighted * coef(e).inv();
            if (power <= grid.root()) small_coeff[int(power)] = value;
            if (e >= 2) prime_powers.emplace_back(power, value);
            if (power > grid.limit() / p) break;
            power *= p;
            weighted *= coef(p).pow(degree);
        }
    }
    std::sort(prime_powers.begin(), prime_powers.end(),
        [](const std::pair<std::int64_t, coef>& a,
           const std::pair<std::int64_t, coef>& b) { return a.first < b.first; });
    BlockPrefix small(grid), whole(grid);
    coef prefix = 0;
    for (std::size_t i = 0; i < small.size(); ++i) {
        std::int64_t x = grid.values()[i];
        if (x <= grid.root()) prefix += small_coeff[int(x)];
        small[i] = prefix;
        whole[i] = power_sum(x, degree);
    }
    BlockPrefix initial = block_exp(small);
    BlockPrefix quotient = dujiao_zak(initial, whole);
    BlockPrefix primes(grid);
    coef correction = 0;
    std::size_t j = 0;
    for (std::size_t i = 0; i < primes.size(); ++i) {
        std::int64_t x = grid.values()[i];
        while (j < prime_powers.size() && prime_powers[j].first <= x)
            correction += prime_powers[j++].second;
        primes[i] = small[i] + quotient[i] - coef(1) - correction;
    }
    return primes;
}

inline BlockPrefix prime_polynomial_via_ln_division(
        const QuotientGrid& grid, const std::vector<coef>& polynomial) {
    if (polynomial.empty())
        throw std::invalid_argument("ln-exp: empty prime polynomial");
    BlockPrefix result(grid);
    for (std::size_t k = 0; k < polynomial.size(); ++k) {
        if (polynomial[k] == coef(0)) continue;
        BlockPrefix monomial = prime_monomial_via_ln_division(grid, int(k));
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] += polynomial[k] * monomial[i];
    }
    return result;
}

// Bell logarithms at p^e (e>=2) complete the log of f; exponentiate it.
// This route constructs the prime part through block division, not Min25.
// Its O(log N) generic block convolutions are not Zak's fast exp algorithm.
template<class Bell>
inline BlockPrefix bell_via_ln_exp(const QuotientGrid& grid,
                                  const std::vector<coef>& polynomial,
                                  Bell bell) {
    BlockPrefix q = prime_polynomial_via_ln_division(grid, polynomial);
    number_theory::Sieve sieve(grid.root());
    std::vector<std::pair<std::int64_t, coef>> events;
    for (int p : sieve.primes) {
        std::int64_t power = p;
        std::vector<coef> a(1), loga(1);
        for (int e = 1; ; ++e) {
            a.push_back(bell(p, e));
            if (e == 1 && a[e] != polynomial_at(polynomial, p))
                throw std::invalid_argument("ln-exp: Bell f(p) disagrees with polynomial");
            coef v = coef(e) * a[e];
            for (int j = 1; j < e; ++j) v -= coef(j) * loga[j] * a[e - j];
            loga.push_back(v * coef(e).inv());
            if (e >= 2) events.emplace_back(power, loga[e]);
            if (power > grid.limit() / p) break;
            power *= p;
        }
    }
    std::sort(events.begin(), events.end(),
        [](const std::pair<std::int64_t, coef>& a,
           const std::pair<std::int64_t, coef>& b) { return a.first < b.first; });
    coef extra = 0;
    std::size_t j = 0;
    for (std::size_t i = 0; i < q.size(); ++i) {
        std::int64_t x = grid.values()[i];
        while (j < events.size() && events[j].first <= x)
            extra += events[j++].second;
        q[i] += extra;
    }
    return block_exp(q);
}

} } // namespace mal::dgf
