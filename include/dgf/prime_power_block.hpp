#pragma once
#include "dirichlet_power.hpp"

namespace mal { namespace dgf {

// Prefix of a function supported on prime powers: g(p^e)=bell(p,e), g(1)=0.
// The caller supplies the prefix of g restricted to ALL primes p<=N;
// iterating only p<=sqrt(N) then adds every remaining p^e (e>=2).
// The output and temporary storage use O(sqrt N) space. Event updates are
// O(1) each, in addition to the caller's prime-prefix construction cost.
template<class Bell>
inline BlockPrefix prime_power_block(const BlockPrefix& prime_prefix, Bell bell) {
    const QuotientGrid& grid = prime_prefix.grid();
    std::int64_t n = grid.limit();
    number_theory::Sieve sieve(grid.root());
    BlockSums blocks(prime_prefix);
    for (int p : sieve.primes) {
        std::int64_t power = p;
        for (int e = 2; power <= n / p; ++e) {
            power *= p;
            std::int64_t upper = power <= grid.root()
                               ? power : n / (n / power);
            coef v = bell(p, e);
            if (v != coef(0)) blocks.add_static(grid.index(upper), v);
        }
    }
    return blocks.to_prefix();
}

} } // namespace mal::dgf
