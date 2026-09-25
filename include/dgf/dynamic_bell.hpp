#pragma once
#include "block_sieve.hpp"
#include "../number_theory/primality.hpp"
#include <algorithm>
#include <cstdint>

namespace mal { namespace dgf {

// Dynamic block sieve for changing one prime's entire Bell series.
// Initialization O(N log log N) with a supplied linear sieve; a change at p
// touches only x >= p in D(N), with O(log_p N) work per touched x.
// The P17465 XOR answer is maintained during updates, so queries are O(1).
class DynamicBellBlock {
    QuotientGrid grid_;
    BlockPrefix sums_;
    series bell_; // only entries at prime powers are read
    std::uint64_t xor_ = 0;
public:
    DynamicBellBlock(const DynamicBellBlock&) = delete;
    DynamicBellBlock& operator=(const DynamicBellBlock&) = delete;
    DynamicBellBlock(DynamicBellBlock&&) = delete;
    DynamicBellBlock& operator=(DynamicBellBlock&&) = delete;
    DynamicBellBlock(number_theory::Sieve sieve, series prime_powers)
        : grid_(int(prime_powers.size()) - 1), sums_(grid_),
          bell_(std::move(prime_powers)) {
        int n = grid_.limit();
        if (int(sieve.spf.size()) != n + 1 || bell_[0] != coef(0))
            throw std::invalid_argument("dynamic Bell: sieve or array limit differs");
        series f(n + 1);
        f[1] = 1;
        coef total = 1;
        std::size_t at = 0;
        const auto& xs = grid_.values();
        if (xs[at] == 1) {
            sums_[at] = total;
            xor_ ^= std::uint64_t(xs[at]) * total.v;
            ++at;
        }
        for (int x = 2; x <= n; ++x) {
            int p = sieve.spf[x], core = x, power = 1;
            do { core /= p; power *= p; } while (core % p == 0);
            f[x] = f[core] * bell_[power];
            total += f[x];
            if (at < xs.size() && xs[at] == x) {
                sums_[at] = total;
                xor_ ^= std::uint64_t(x) * total.v;
                ++at;
            }
        }
    }
    const QuotientGrid& grid() const { return grid_; }
    const BlockPrefix& block() const { return sums_; }
    std::uint64_t answer() const { return xor_; }

    void update(int p, const std::vector<coef>& new_values) {
        if (p < 2 || p > grid_.limit() || !number_theory::is_prime(p))
            throw std::invalid_argument("dynamic Bell: p must be prime <= N");
        std::vector<int> powers(1, 1);
        for (int q = p; q <= grid_.limit(); ) {
            powers.push_back(q);
            if (q > grid_.limit() / p) break;
            q *= p;
        }
        int emax = int(powers.size()) - 1;
        if (int(new_values.size()) != emax)
            throw std::invalid_argument("dynamic Bell: wrong Bell-series length");
        std::vector<coef> ratio(emax + 1);
        ratio[0] = 1;
        int first = emax + 1;
        for (int e = 1; e <= emax; ++e) {
            ratio[e] = new_values[e - 1];
            for (int j = 1; j <= e; ++j)
                ratio[e] -= bell_[powers[j]] * ratio[e - j];
            if (ratio[e] != coef(0) && first > e) first = e;
        }
        if (first <= emax) {
            const auto& xs = grid_.values();
            std::size_t begin = std::lower_bound(xs.begin(), xs.end(), powers[first]) - xs.begin();
            // Descending x guarantees all F(floor(x/p^j)) still use OLD data.
            for (std::size_t i = xs.size(); i-- > begin; ) {
                int x = xs[i];
                coef updated = sums_[i];
                for (int e = first; e <= emax && powers[e] <= x; ++e)
                    if (ratio[e] != coef(0))
                        updated += ratio[e] * sums_.at(x / powers[e]);
                xor_ ^= std::uint64_t(x) * sums_[i].v;
                sums_[i] = updated;
                xor_ ^= std::uint64_t(x) * updated.v;
            }
        }
        for (int e = 1; e <= emax; ++e) bell_[powers[e]] = new_values[e - 1];
    }
};

} } // namespace mal::dgf
