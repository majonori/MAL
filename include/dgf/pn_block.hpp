#pragma once
#include "prime_power_block.hpp"

namespace mal { namespace dgf {

// Multiplicative PN factor h: h(1)=1, h(p)=0, h(p^e)=bell(p,e) for e>=2.
// Only powerful numbers can have nonzero values. Enumerate distinct prime
// powers once in increasing-prime order, and accumulate into quotient blocks.
// O(sqrt N) storage. The enumeration cost depends on local Bell sparsity.
template<class Bell>
inline BlockPrefix pn_block(const QuotientGrid& grid, Bell bell) {
    const std::int64_t n = grid.limit();
    number_theory::Sieve sieve(grid.root());
    BlockSums blocks(grid);
    const auto& ps = sieve.primes;
    auto visit = [&](auto&& self, std::size_t start,
                     std::int64_t value, coef weight) -> void {
        std::int64_t upper = value <= grid.root()
                           ? value : n / (n / value);
        blocks.add_static(grid.index(upper), weight);
        std::int64_t remaining = n / value;
        for (std::size_t i = start; i < ps.size(); ++i) {
            int p = ps[i];
            if (1LL * p * p > remaining) break;
            std::int64_t power = 1LL * p * p;
            for (int e = 2; power <= remaining; ++e) {
                coef local = bell(p, e);
                if (local != coef(0))
                    self(self, i + 1, value * power, weight * local);
                if (power > remaining / p) break;
                power *= p;
            }
        }
    };
    visit(visit, 0, 1, coef(1));
    return blocks.to_prefix();
}

} } // namespace mal::dgf
