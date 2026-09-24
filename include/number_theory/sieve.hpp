#pragma once
#include <vector>
#include <utility>
#include <stdexcept>

namespace mal { namespace number_theory {

// O(N) linear sieve. Reusable by the DGF module and standalone number theory.
struct Sieve {
    std::vector<int> spf, primes;
    explicit Sieve(int n);
    bool prime(int x) const;
    std::vector<std::pair<int, int>> factor(int x) const;
};

inline Sieve::Sieve(int n) : spf(n >= 0 ? n + 1 : 0) {
    if (n < 0) throw std::invalid_argument("sieve: negative limit");
    for (int i = 2; i <= n; ++i) {
        if (!spf[i]) spf[i] = i, primes.push_back(i);
        for (int p : primes) {
            if (p > spf[i] || 1LL * i * p > n) break;
            spf[i * p] = p;
        }
    }
}
inline bool Sieve::prime(int x) const {
    if (x < 0 || x >= int(spf.size())) throw std::out_of_range("sieve: outside table");
    return x >= 2 && spf[x] == x;
}
inline std::vector<std::pair<int, int>> Sieve::factor(int x) const {
    if (x < 1 || x >= int(spf.size())) throw std::out_of_range("sieve: outside table");
    std::vector<std::pair<int, int>> result;
    while (x > 1) {
        int p = spf[x], e = 0;
        do { x /= p; ++e; } while (x > 1 && spf[x] == p);
        result.emplace_back(p, e);
    }
    return result;
}

inline Sieve make_sieve(int n) { return Sieve(n); }

// O(N), in increasing index order using the smallest prime factor.
inline std::vector<int> totients(const Sieve& s) {
    int n = int(s.spf.size()) - 1;
    std::vector<int> phi(n + 1);
    if (n >= 1) phi[1] = 1;
    for (int i = 2; i <= n; ++i) {
        int p = s.spf[i], m = i / p;
        phi[i] = (m % p == 0) ? phi[m] * p : phi[m] * (p - 1);
    }
    return phi;
}
inline std::vector<int> mobius(const Sieve& s) {
    int n = int(s.spf.size()) - 1;
    std::vector<int> mu(n + 1);
    if (n >= 1) mu[1] = 1;
    for (int i = 2; i <= n; ++i) {
        int p = s.spf[i], m = i / p;
        mu[i] = (m % p == 0) ? 0 : -mu[m];
    }
    return mu;
}

namespace detail {
__attribute__((used)) auto make_sieve_kept = &make_sieve;
__attribute__((used)) auto sieve_prime_kept = &Sieve::prime;
__attribute__((used)) auto sieve_factor_kept = &Sieve::factor;
__attribute__((used)) auto totients_kept = &totients;
__attribute__((used)) auto mobius_kept = &mobius;
} // namespace detail

} } // namespace mal::number_theory
