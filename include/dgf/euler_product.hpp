#pragma once
#include "block_sieve.hpp"

namespace mal { namespace dgf {

// A factor 1 + sum_{m>=2} a_m [m], where [m] is the Dirichlet monomial.
// Supply factors in strictly increasing order of their least supported m.
struct EulerFactor {
    std::vector<std::pair<std::int64_t, coef>> terms;
};

// Reverse Euler-product stage. The caller supplies the block sieve of the
// already combined large factors (the prime-prefix sieve, when applicable,
// is a separate FIRST stage). No multiplicativity is required.
//
// If old f(n)=0 for 2<=n<next_support, each factor term at m splits at
// B=max(t, floor(sqrt(N/m))): enumerate old n=1 and next_support<=n<=B;
// for n>B, enumerate target quotient blocks and query the old prefix sums.
// A Fenwick tree holds sums of disjoint quotient intervals. All deltas for a
// factor are collected BEFORE applying any of them, so cross terms between
// monomials of the same factor are never accidentally included.
// Cost: O(|D(N)| + sum_{factors,terms m}
//       (max(0,B-next_support+1)+N/(m*(B+1))+1)*log |D(N)|).
// This formula, together with sparse factors / prime distribution, is the
// condition for an N^(2/3) bound; it is NOT a bound for arbitrary input.
inline BlockPrefix reverse_euler_product(const BlockPrefix& large_tail,
                                         const std::vector<EulerFactor>& small,
                                         int tail_support) {
    const QuotientGrid& grid = large_tail.grid();
    const std::int64_t n = grid.limit();
    const int root = grid.root();
    if (tail_support < 2 || tail_support > root + 1 ||
        large_tail.at(1) != coef(1))
        throw std::invalid_argument("Euler product: invalid tail support or unit");
    std::vector<int> least(small.size());
    for (std::size_t i = 0; i < small.size(); ++i) {
        if (small[i].terms.empty()) throw std::invalid_argument("Euler product: empty factor");
        std::int64_t previous = 1;
        for (const auto& term : small[i].terms) {
            if (term.first <= previous || term.first > n || term.second == coef(0))
                throw std::invalid_argument("Euler product: invalid or unsorted term");
            previous = term.first;
        }
        if (small[i].terms.front().first > root)
            throw std::invalid_argument("Euler product: supports must increase up to sqrt(N)");
        least[i] = int(small[i].terms.front().first);
        if (i && least[i] <= least[i - 1])
            throw std::invalid_argument("Euler product: supports must increase");
    }
    if (!small.empty() && tail_support <= least.back())
        throw std::invalid_argument("Euler product: tail support must be larger");
    for (int x = 2; x < tail_support; ++x)
        if (large_tail.point(x) != coef(0))
            throw std::invalid_argument("Euler product: tail has small support");

    const auto& xs = grid.values();
    BlockSums blocks(large_tail);

    for (std::size_t i = small.size(); i-- > 0; ) {
        int next_support = i + 1 < small.size() ? least[i + 1] : tail_support;
        std::vector<std::pair<int, coef>> pending;
        for (const auto& term : small[i].terms) {
            std::int64_t m = term.first;
            coef weight = term.second;
            int b = int(std::min<std::int64_t>(least[i], n / m));
            while (1LL * (b + 1) * (b + 1) <= n / m) ++b;
            // The term [m] always maps the unit to m.
            pending.emplace_back(int(std::lower_bound(xs.begin(), xs.end(), m) - xs.begin()), weight);
            for (int source = next_support; source <= b; ++source) {
                coef value = blocks.small_point(source);
                if (value == coef(0)) continue;
                std::int64_t target = m * source;
                int j = int(std::lower_bound(xs.begin(), xs.end(), target) - xs.begin());
                pending.emplace_back(j, weight * value);
            }
            // Only blocks with upper endpoint >=m(B+1) can contain a source >B.
            long long first_target = 1LL * m * (b + 1);
            if (first_target > n) continue;
            auto begin = std::lower_bound(xs.begin(), xs.end(), first_target);
            for (auto it = begin; it != xs.end(); ++it) {
                int j = int(it - xs.begin());
                std::int64_t upper = *it / m;
                std::int64_t lower = j ? xs[j - 1] / m : 0;
                if (upper <= b) continue;
                coef value = blocks.prefix_at(upper)
                           - blocks.prefix_at(std::max<std::int64_t>(b, lower));
                if (value != coef(0)) pending.emplace_back(j, weight * value);
            }
        }
        for (const auto& change : pending)
            blocks.add_block(change.first, change.second);
    }
    return blocks.to_prefix();
}

} } // namespace mal::dgf
